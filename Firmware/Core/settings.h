/*
 * settings.h
 *
 *  Created on: Feb 22, 2025
 *      Author: Kikkiu
 */

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include "stdint.h"

#define SERVO_UPPER_ENDSTOP 135
#define SERVO_LOWER_ENDSTOP 0
extern uint32_t servo_max;
extern uint32_t servo_min;
#define SERVO_ABS_BRAKE_RATE 4	// per cycle

#define BRAKE_THRESHOLD (servo_max - servo_min) * 0.75 + servo_min
#define BRAKE_ANGLE_MAX 38
#define BRAKE_ANGLE_OFFSET 67

#define MAX_SPEED 150	// kmh

#define LIGHT_FLASHER_TIME 225	// ms
#define LOW_BATTERY_THRESHOLD 6900 // mV
#define CHARGED_BATTERY_THRESHOLD 8300	// mV
#define BATTERY_CHARGE_WIDTH 11

extern uint32_t brake_light_pwm_period;
extern uint8_t servo_enabled;
extern uint8_t servo_hold;
extern uint8_t abs_enabled;
extern uint8_t screen_brightness;

#endif /* INC_SETTINGS_H_ */
