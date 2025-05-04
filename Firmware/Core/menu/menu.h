/*
 * menu.h
 *
 *  Created on: Mar 2, 2025
 *      Author: Kikkiu
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include "../settings.h"
#include "../sh1106/sh1106.h"
#include "../bitmaps/bitmaps.h"
#include <stdint.h>

void MENU_Set(uint32_t num);
void MENU_ButtonShortPress();
void MENU_ButtonLongPress();
uint8_t MENU_Update();
void MENU_UpdateData(uint32_t _raw_brake_lever_angle, uint32_t _brake_lever_angle, uint32_t _servo_angle, uint32_t _batt_voltage_mv);
void MENU_UpdateDebug(uint32_t kmh, uint32_t period, uint32_t last_capture_value, uint32_t last_capture_time, float current_freq, float avg_freq);


#endif /* INC_MENU_H_ */
