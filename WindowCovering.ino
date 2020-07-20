/*
 * WindowCovering.ino
 *
 *  Created on: 2020-06-25
 *      Author: jmbwell
 *
 * HAP section 8.45 window covering
 * An accessory that contains a window covering that raises and lowers and tilts up and down.
 *
 */

#include <Arduino.h>
#include <Servo.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"

#include <espconn.h>

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);


// Tilt servo motor control pin
#define PIN_TILT_SERVO 5

// Tilt servo controller
Servo tilt_servo;

// Tilt servo value limits — dependent upon the model servo in use
float servo_min_value = 0;
float servo_max_value = 270;

// Calibrated limits - the range of the blinds, from "tilted fully upward" to "tilted fully downward"
float servo_min_limit = 0;
float servo_max_limit = 230;


// Position motor control pin (raise/lower)
#define PIN_POSITION_MOTOR 0


// Timeout for raising and lowering before reporting an obstruction
#define TARGET_ACQUISITION_TIMEOUT_SECS 10


//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_position_state;
extern "C" homekit_characteristic_t cha_current_position;
extern "C" homekit_characteristic_t cha_target_position;
extern "C" homekit_characteristic_t cha_name;
extern "C" homekit_characteristic_t cha_obstruction_detected;
extern "C" homekit_characteristic_t cha_current_horizontal_tilt_angle;
extern "C" homekit_characteristic_t cha_target_horizontal_tilt_angle;

// You could implement these too if you wanted
// extern "C" homekit_characteristic_t cha_hold_position;
// extern "C" homekit_characteristic_t cha_current_vertical_tilt_angle;
// extern "C" homekit_characteristic_t cha_target_vertical_tilt_angle;


// For reporting heap usage on the serial output every 5 seconds
static uint32_t next_heap_millis = 0;


// Called when getting up/down or open/closed position
homekit_value_t cha_current_position_getter() {
	return cha_current_position.value;
}


// Called when getting horizontal tilt angle
homekit_value_t cha_current_horizontal_tilt_angle_getter() {

	// Angle reported by the servo library
	int servo_angle = tilt_servo.read();
	LOG_D("Servo angle (arduino): %i", servo_angle);

	// Real servo angle
	int real_angle = map(servo_angle, 0, 180, servo_min_value, servo_max_value);
	LOG_D("Servo angle (real): %i", real_angle);

	// Angle mapped from the servo's range (specified above) to HomeKit's range (-90 to 90)
	int homekit_angle = map(real_angle, servo_min_limit, servo_max_limit, cha_current_horizontal_tilt_angle.min_value[0], cha_current_horizontal_tilt_angle.max_value[0]);

	// Set it in our internal characteristic
	cha_current_horizontal_tilt_angle.value.int_value = homekit_angle;
	LOG_D("HomeKit angle: %i", cha_current_horizontal_tilt_angle.value.int_value);

	// Return it to the caller
	return cha_current_horizontal_tilt_angle.value;
}


// Called when setting horizontal tilt angle
void cha_target_horizontal_tilt_angle_setter(const homekit_value_t value) {

	// Angle requested by HomeKit
	int homekit_angle = value.int_value;
	cha_target_horizontal_tilt_angle.value.int_value = homekit_angle;
	LOG_D("Target HomeKit angle: %i (%i)", homekit_angle, value.format);

	// Real servo angle
	int real_angle = map(homekit_angle, cha_target_horizontal_tilt_angle.min_value[0], cha_target_horizontal_tilt_angle.max_value[0], servo_min_limit, servo_max_limit);
	LOG_D("Target servo angle (real): %i", real_angle);

	// Angle mapped from HomeKit range (-90 to 90) to servo range (defined at the top)
	int servo_angle = map(real_angle, servo_min_value, servo_max_value, 0, 180);
	LOG_D("Target servo angle: %i", servo_angle);

	// Command the servo
	tilt_servo.write(servo_angle);

	// New angle reported by the getter
	homekit_value_t new_angle = cha_current_horizontal_tilt_angle_getter();

	// Notify HomeKit clients of the change
	homekit_characteristic_notify(&cha_current_horizontal_tilt_angle, new_angle);
}


// Called when setting up/down or open/closed position
void cha_target_position_setter(const homekit_value_t value) {

	// HomeKit target position value
	uint position = value.uint8_value;
	cha_target_position.value.uint8_value = position;
	LOG_D("Target position: %i%%", position);

	// If the blinds are fully closed, tilt them to 0 degrees HomeKit (90 degrees Servo) before opening them
	if (cha_current_position.value.uint8_value == 0) {
		homekit_value_t horizontal_tilt = HOMEKIT_INT_CPP(0);
		cha_target_horizontal_tilt_angle_setter(horizontal_tilt);
	}

	// Run the motor in the required direction until it reaches the required position...
	// TODO: determine required direction, run until a stop switch is closed, etc.
	// For now, just set the value so HomeKit won't complain
	cha_current_position.value.uint8_value = position;

	// See whether it was able to get there
	for (uint i = 0; i < TARGET_ACQUISITION_TIMEOUT_SECS; i++) {
		if (cha_current_position.value.uint8_value == position) {
			homekit_characteristic_notify(&cha_current_position, cha_current_position.value);
			LOG_D("Current position: %u%%", cha_current_position.value.uint8_value);
			cha_obstruction_detected.value.bool_value = false;
			homekit_characteristic_notify(&cha_obstruction_detected, cha_obstruction_detected.value);
			break;
		} else {
			delay(1000);
		}
		// Uh-oh
		cha_obstruction_detected.value.bool_value = true;
		homekit_characteristic_notify(&cha_obstruction_detected, cha_obstruction_detected.value);
	}
	
	// If the blinds are now fully closed, tilt them to -90 degrees HomeKit (0 degrees Servo)
	if (cha_current_position.value.uint8_value == 0) {
		homekit_value_t horizontal_tilt = HOMEKIT_INT_CPP(-90);
		cha_target_horizontal_tilt_angle_setter(horizontal_tilt);
	}

}


void my_homekit_setup() {

	// Turn on the servo
	tilt_servo.attach(PIN_TILT_SERVO);

	// Set the setters and getters
	cha_target_position.setter = cha_target_position_setter;
	cha_current_position.getter = cha_current_position_getter;
	cha_target_horizontal_tilt_angle.setter = cha_target_horizontal_tilt_angle_setter;
	cha_current_horizontal_tilt_angle.getter = cha_current_horizontal_tilt_angle_getter;

	arduino_homekit_setup(&config);
}

void my_homekit_loop() {
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		// show heap info every 5 seconds
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}


void setup() {
  Serial.begin(115200);
  wifi_connect(); // in wifi_info.h
  //homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
  my_homekit_setup();
}

void loop() {
  my_homekit_loop();
  delay(10);
}
