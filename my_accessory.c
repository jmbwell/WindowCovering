/*
 * my_accessory.c
 * Define the accessory in C language using the Macro in characteristics.h
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 */

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
}

// Window Covering (HAP section 8.45)
// Required Characteristics:
// - CURRENT_POSITION
// - TARGET_POSITION
// - POSITION_STATE

// Optional Characteristics:
// - NAME
// - HOLD_POSITION
// - TARGET_HORIZONTAL_TILT_ANGLE
// - TARGET_VERTICAL_TILT_ANGLE
// - CURRENT_HORIZONTAL_TILT_ANGLE
// - CURRENT_VERTICAL_TILT_ANGLE
// - OBSTRUCTION_DETECTED

// format: uint8; HAP section 9.73; 0 = closing, 1 = opening, 2 = stopped
homekit_characteristic_t cha_position_state = HOMEKIT_CHARACTERISTIC_(POSITION_STATE, 2);

// format: uint8; HAP section 9.27; 0 (closed) to 100 (open), percentage
homekit_characteristic_t cha_current_position = HOMEKIT_CHARACTERISTIC_(CURRENT_POSITION, 0);

// format: uint8; HAP section 9.117; 0 (closed) to 100 (open), percentage
homekit_characteristic_t cha_target_position = HOMEKIT_CHARACTERISTIC_(TARGET_POSITION, 0);

// format: string; HAP section 9.62; max length 64
homekit_characteristic_t cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "WindowCovering-01");

// format: bool; HAP section 9.43; 0 = ignored, 1 = hold
homekit_characteristic_t cha_hold_position = HOMEKIT_CHARACTERISTIC_(HOLD_POSITION, 0);

// format: int; HAP section 9.24; range: -90 to 90, where -90 is all the way up and 90 is all the way down.
homekit_characteristic_t cha_current_horizontal_tilt_angle = HOMEKIT_CHARACTERISTIC_(CURRENT_HORIZONTAL_TILT_ANGLE, 0);

// format: int; HAP section 9.115; range: -90 to 90, where -90 is all the way up and 90 is all the way down.
homekit_characteristic_t cha_target_horizontal_tilt_angle = HOMEKIT_CHARACTERISTIC_(TARGET_HORIZONTAL_TILT_ANGLE, 0);

// format: int; HAP section 9.28; range: -90 to 90, where -90 is all the way left and 90 is all the way right.
// homekit_characteristic_t cha_current_vertical_tilt_angle = HOMEKIT_CHARACTERISTIC_(CURRENT_VERTICAL_TILT_ANGLE, 0);

// format: int; HAP section 9.123; range: -90 to 90, where -90 is all the way left and 90 is all the way right.
// homekit_characteristic_t cha_target_vertical_tilt_angle = HOMEKIT_CHARACTERISTIC_(TARGET_VERTICAL_TILT_ANGLE, 0);

// format: bool; HAP section 9.65; 0 = no obstruction, 1 = obstruction detected
homekit_characteristic_t cha_obstruction_detected = HOMEKIT_CHARACTERISTIC_(OBSTRUCTION_DETECTED, 0);

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_window_covering, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "WindowCovering-01"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Arduino HomeKit"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0123456"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266/ESP32"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
            NULL
        }),
		HOMEKIT_SERVICE(WINDOW_COVERING, .primary=true, .characteristics=(homekit_characteristic_t*[]){
			&cha_position_state,
			&cha_current_position,
			&cha_target_position,
			&cha_name,
			// &cha_hold_position,
			&cha_current_horizontal_tilt_angle,
			&cha_target_horizontal_tilt_angle,
			// &cha_current_vertical_tilt_angle,
			// &cha_target_vertical_tilt_angle,
			&cha_obstruction_detected,
			NULL
		}),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
		.accessories = accessories,
		.password = "111-11-111"
};
