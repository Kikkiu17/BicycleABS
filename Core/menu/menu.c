/*
 * menu.c
 *
 *  Created on: Mar 2, 2025
 *      Author: Kikkiu
 */

#include "menu.h"
#include "stm32f1xx_hal.h"
#include <inttypes.h>
#include <stdio.h>

uint32_t arrow_y = 2;
uint8_t number_of_elements = 4;
uint32_t screen = 0;
uint32_t item_selected = 0;
uint32_t submenu_selected = 0;

uint32_t raw_brake_angle = 0;
uint32_t brake_angle = 0;
uint32_t s_angle = 0;
uint32_t battery_int_v = 0;
uint32_t battery_dec_v = 0;

uint32_t debug_kmh = 0;
uint32_t debug_period = 0;
uint32_t debug_last_capture_value = 0;
uint32_t debug_last_capture_time = 0;
float debug_current_freq = 0.00;
float debug_avg_freq = 0.00;

uint8_t list = 0;


void MENU_Set(uint32_t num)
{
	screen = num;
}


void MENU_ButtonShortPress()
{
	if (((item_selected >= number_of_elements - 1) && screen != 1) || ((submenu_selected >= number_of_elements - 1) && screen == 1))
	{
		if (screen == 1)
		{
			submenu_selected = 0;
			list = 0;
		}

		if (submenu_selected == 13)
			submenu_selected = 1;
		else if (submenu_selected == 20)
			submenu_selected = 2;
		else if (submenu_selected == 30)
			submenu_selected = 3;
		else if (submenu_selected == 40)
			submenu_selected = 4;

		item_selected = 0;
		arrow_y = 2;
	}
	else
	{
		arrow_y += 16;

		if (submenu_selected == 1 && item_selected == 3)
		{
			submenu_selected = 10;	// settings page 2
			arrow_y = 2;
		}
		else if (submenu_selected == 10 && item_selected == 7)
		{
			submenu_selected = 11;	// settings page 3
			arrow_y = 2;
		}
		else if (submenu_selected == 11 && item_selected == 11)
		{
			submenu_selected = 12;	// settings page 4
			arrow_y = 2;
		}
		else if (submenu_selected == 12 && item_selected == 15)
		{
			submenu_selected = 13;	// settings page 5
			arrow_y = 2;
		}
		else if (submenu_selected == 2 && item_selected == 3)
		{
			submenu_selected = 20;	// lights page 2
			arrow_y = 2;
		}
		else if (submenu_selected == 3 && item_selected == 2)
		{
			submenu_selected = 30;	// data page 2
			arrow_y = 2;
		}
		else if (submenu_selected == 3 && screen == 1)
		{
			list = 1;	// menu page 2
			arrow_y = 2;
		}
		else if (submenu_selected == 4 && item_selected == 3)
		{
			submenu_selected = 40;	// debug page 2
			arrow_y = 2;
		}

		if (screen == 1)
			submenu_selected++;
		else
			item_selected++;
	}
}


void MENU_ButtonLongPress()
{
	if (screen == 1)
	{
		if (submenu_selected == 0)
		{
			screen = 0;
			return;
		}

		item_selected = 0;
		arrow_y = 2;
		screen = 2;
	}
	else if (screen == 2)
	{
		if (item_selected == 0)
		{
			list = 0;
			submenu_selected = 0;
			arrow_y = 2 + 16 * submenu_selected;
			screen = 1;
		}

		if (submenu_selected == 1)
		{
			// settings page 1
			if (item_selected == 1)
				servo_enabled = !servo_enabled;
			else if (item_selected == 2)
				abs_enabled = !abs_enabled;
			else if (item_selected == 3)
				servo_hold = !servo_hold;
		}
		else if (submenu_selected == 10)
		{
			if (item_selected == 5)
			{
				if (screen_brightness < 255)
				{
					screen_brightness += 30;
					SH1106_SetBrightness(screen_brightness);
				}
			}
			else if (item_selected == 6)
			{
				if (screen_brightness > 15)
				{
					screen_brightness -= 30;
					SH1106_SetBrightness(screen_brightness);
				}
			}
		}
		else if (submenu_selected == 11)
		{
			// settings page 3
			if (item_selected == 9)
				servo_max += 2;
			else if (item_selected == 10)
				servo_max -= 2;
		}
		else if (submenu_selected == 12)
		{
			// settings page 4
			if (item_selected == 13)
				servo_min += 2;
			else if (item_selected == 14)
				servo_min -= 2;
		}
		else if (submenu_selected == 13)
		{
			// settings page 5
			if (item_selected == 17)
			{
				if ((TIM2->PSC + 1) * 2 <= 60000)
				{
					TIM2->PSC = (TIM2->PSC + 1) * 2 - 1;	// multiply current PSC by 2
					TIM2->EGR = TIM2->EGR | 0x01;	// update timer
				}
			}
			else if (item_selected == 18)
			{
				if ((TIM2->PSC + 1) / 2 >= 1875)
				{
					TIM2->PSC = (TIM2->PSC + 1) / 2 - 1;	// divide current PSC by 2
					TIM2->EGR = TIM2->EGR | 0x01;	// update timer
				}
			}
		}
		else if (submenu_selected == 20)
		{
			// lights page 2
			if (item_selected == 5)
				brake_light_pwm_period += 1000;
			else if (item_selected == 6)
			{
				if (brake_light_pwm_period <= 1000)
					brake_light_pwm_period = 0;
				else
					brake_light_pwm_period -= 1000;
			}
		}
	}
}


char item[25];
uint8_t MENU_Update()
{
	uint8_t draw_selection_arrow = 1;

	if (screen == 1)
	{
		number_of_elements = 5;

		if (list == 0)
		{
			SH1106_WriteChars(12, 1, "Home", 4, Arial_12pt, false);
			SH1106_WriteChars(12, 17, "Impostazioni", 12, Arial_12pt, false);
			SH1106_WriteChars(12, 33, "Luci", 4, Arial_12pt, false);
			SH1106_WriteChars(12, 49, "Dati", 6, Arial_12pt, false);

			if (submenu_selected == 3)
			{
				draw_selection_arrow = 0;
				SH1106_DrawBitmap(down_arrow_img, 1, 59, 7, 5, false);
			}
			else
				SH1106_DrawBitmap(down_arrow_img, 1, 54, 7, 5, false);
		}
		else if (list == 1)
		{
			// page 2 of main list
			SH1106_WriteChars(12, 1, "Debug", 6, Arial_12pt, false);
		}

	}
	else if (screen == 2)
	{
		if (submenu_selected == 1 || submenu_selected == 10 || submenu_selected == 11 || submenu_selected == 12 || submenu_selected == 13)
		{
			// -------------------- SETTINGS --------------------
			number_of_elements = 19;

			if (submenu_selected == 1)
			{
				SH1106_WriteChars(12, 1, "Indietro", 8, Arial_12pt, false);
				if (servo_enabled == 1)
					SH1106_WriteChars(12, 17, "Servo ON", 8, Arial_12pt, false);
				else
					SH1106_WriteChars(12, 17, "Servo OFF", 9, Arial_12pt, false);

				if (abs_enabled == 1)
					SH1106_WriteChars(12, 33, "ABS ON", 6, Arial_12pt, false);
				else
					SH1106_WriteChars(12, 33, "ABS OFF", 7, Arial_12pt, false);

				if (servo_hold == 1)
					SH1106_WriteChars(12, 49, "Servo HOLD", 10, Arial_12pt, false);
				else
					SH1106_WriteChars(12, 49, "Servo NORMALE", 13, Arial_12pt, false);

				if (item_selected == 3)
				{
					draw_selection_arrow = 0;
					SH1106_DrawBitmap(down_arrow_img, 1, 59, 7, 5, false);
				}
				else
					SH1106_DrawBitmap(down_arrow_img, 1, 54, 7, 5, false);
			}
			else if (submenu_selected == 10)
			{
				sprintf(item, "Lum schermo %d", screen_brightness);
				SH1106_WriteChars(12, 1, item, 15, Arial_12pt, false);
				SH1106_WriteChars(12, 17, "+ 30", 9, Arial_12pt, false);
				SH1106_WriteChars(12, 33, "- 30", 9, Arial_12pt, false);
				if (item_selected == 7)
				{
					draw_selection_arrow = 0;
					SH1106_DrawBitmap(down_arrow_img, 1, 59, 7, 5, false);
				}
				else
					SH1106_DrawBitmap(down_arrow_img, 1, 54, 7, 5, false);
			}
			else if (submenu_selected == 11)
			{
				sprintf(item, "Servo max: %" PRIu32, servo_max);
				SH1106_WriteChars(12, 1, item, 14, Arial_12pt, false);
				SH1106_WriteChars(12, 17, "+ 2 gradi", 9, Arial_12pt, false);
				SH1106_WriteChars(12, 33, "- 2 gradi", 9, Arial_12pt, false);
				if (item_selected == 11)
				{
					draw_selection_arrow = 0;
					SH1106_DrawBitmap(down_arrow_img, 1, 59, 7, 5, false);
				}
				else
					SH1106_DrawBitmap(down_arrow_img, 1, 54, 7, 5, false);
			}
			else if (submenu_selected == 12)
			{
				sprintf(item, "Servo min %" PRIu32, servo_min);
				SH1106_WriteChars(12, 1, item, 13, Arial_12pt, false);
				SH1106_WriteChars(12, 17, "+ 2 gradi", 9, Arial_12pt, false);
				SH1106_WriteChars(12, 33, "- 2 gradi", 9, Arial_12pt, false);
				if (item_selected == 15)
				{
					draw_selection_arrow = 0;
					SH1106_DrawBitmap(down_arrow_img, 1, 59, 7, 5, false);
				}
				else
					SH1106_DrawBitmap(down_arrow_img, 1, 54, 7, 5, false);
			}
			else if (submenu_selected == 13)
			{
				sprintf(item, "TIM2 PSC %" PRIu32, TIM2->PSC + 1);
				SH1106_WriteChars(12, 1, item, 14, Arial_12pt, false);
				SH1106_WriteChars(12, 17, "PSC *= 2", 8, Arial_12pt, false);
				SH1106_WriteChars(12, 33, "PSC /= 2", 8, Arial_12pt, false);
				sprintf(item, "TIM2 CNT %" PRIu32, TIM2->CNT);
				SH1106_WriteChars(12, 49, item, 14, Arial_12pt, false);
			}
		}
		if (submenu_selected == 2 ||  submenu_selected == 20)
		{
			// -------------------- LIGHTS --------------------
			number_of_elements = 7;

			if (submenu_selected == 2)
			{
				SH1106_WriteChars(12, 1, "Indietro", 8, Arial_12pt, false);
				sprintf(item, "Lampeggio: %d ms", LIGHT_FLASHER_TIME);
				SH1106_WriteChars(12, 17, item, 17, Arial_12pt, false);
				sprintf(item, "Freq PWM: %" PRIu32 " Hz", 72000000 / TIM1->PSC / TIM1->ARR);
				SH1106_WriteChars(12, 33, item, 16, Arial_12pt, false);

				if (item_selected == 3)
				{
					draw_selection_arrow = 0;
					SH1106_DrawBitmap(down_arrow_img, 1, 59, 7, 5, false);
				}
				else
					SH1106_DrawBitmap(down_arrow_img, 1, 54, 7, 5, false);
			}
			else
			{
				sprintf(item, "Duty cycle %" PRIu32 "%%", brake_light_pwm_period * 100 / 20000);
				SH1106_WriteChars(12, 1, item, 19, Arial_12pt, false);
				SH1106_WriteChars(12, 17, "+ 5%%", 4, Arial_12pt, false);
				SH1106_WriteChars(12, 33, "- 5%%", 4, Arial_12pt, false);
			}

		}
		else if (submenu_selected == 3 || submenu_selected == 30)
		{
			// -------------------- DATA --------------------
			number_of_elements = 5;

			if (submenu_selected == 3)
			{
				char decimal_battery_string[2];
				sprintf(item, "%" PRIu32, battery_dec_v);
				leftPadding(decimal_battery_string, item, 2, '0');
				sprintf(item, "Batteria: %" PRIu32 ".%s V", battery_int_v, decimal_battery_string);

				SH1106_WriteChars(12, 1, "Indietro", 8, Arial_12pt, false);
				SH1106_WriteChars(12, 17, item, 15, Arial_12pt, false);
				sprintf(item, "Freno raw %" PRIu32 " gradi", raw_brake_angle);
				SH1106_WriteChars(12, 33, item, 19, Arial_12pt, false);

				SH1106_DrawBitmap(down_arrow_img, 12, 54, 14, 8, false);
			}
			else
			{
				sprintf(item, "Freno %" PRIu32 " gradi", brake_angle);
				SH1106_WriteChars(12, 1, item, 15, Arial_12pt, false);
				sprintf(item, "Servo %" PRIu32 " gradi", s_angle);
				SH1106_WriteChars(12, 17, item, 15, Arial_12pt, false);
			}
		}
		else if (submenu_selected == 4 || submenu_selected == 40)
		{
			// -------------------- DEBUG --------------------
			number_of_elements = 12;

			if (submenu_selected == 4)
			{
				sprintf(item, "vel %" PRIu32 " kmh", debug_kmh);
				SH1106_WriteChars(12, 1, item, 10, Arial_12pt, false);
				sprintf(item, "period %" PRIu32, debug_period);
				SH1106_WriteChars(12, 17, item, 12, Arial_12pt, false);
				sprintf(item, "last capture %" PRIu32, debug_last_capture_value);
				SH1106_WriteChars(12, 33, item, 19, Arial_12pt, false);
				sprintf(item, "last c time %" PRIu32, debug_last_capture_time);
				SH1106_WriteChars(12, 49, item, 19, Arial_12pt, false);

				if (item_selected == 3)
				{
					draw_selection_arrow = 0;
					SH1106_DrawBitmap(down_arrow_img, 1, 59, 7, 5, false);
				}
				else
					SH1106_DrawBitmap(down_arrow_img, 1, 54, 7, 5, false);
			}
			else if (submenu_selected == 40)
			{
				char dec_cur_freq_str[2];
				char dec_avg_freq_str[2];

				uint32_t int_cur_freq = debug_current_freq;
				uint32_t dec_cur_freq = debug_current_freq * 100 - int_cur_freq * 100;
				uint32_t int_avg_freq = debug_avg_freq;
				uint32_t dec_avg_freq = debug_avg_freq * 100 - int_avg_freq * 100;

				sprintf(item, "%" PRIu32, dec_cur_freq);
				leftPadding(dec_cur_freq_str, item, 2, '0');
				sprintf(item, "%" PRIu32, dec_avg_freq);
				leftPadding(dec_avg_freq_str, item, 2, '0');


				sprintf(item, "vel %" PRIu32 " kmh", debug_kmh);
				SH1106_WriteChars(12, 1, item, 10, Arial_12pt, false);
				sprintf(item, "period %" PRIu32, debug_period);
				SH1106_WriteChars(12, 17, item, 12, Arial_12pt, false);
				sprintf(item, "cur freq %" PRIu32 ".%s", int_cur_freq, dec_cur_freq_str);
				SH1106_WriteChars(12, 33, item, 13, Arial_12pt, false);
				sprintf(item, "avg freq %" PRIu32 ".%s", int_avg_freq, dec_avg_freq_str);
				SH1106_WriteChars(12, 49, item, 13, Arial_12pt, false);

				if (item_selected == 7)
				{
					draw_selection_arrow = 0;
					SH1106_DrawBitmap(down_arrow_img, 1, 59, 7, 5, false);
				}
				else
					SH1106_DrawBitmap(down_arrow_img, 1, 54, 7, 5, false);
			}
		}
	}

	if (screen != 0 && draw_selection_arrow)
		SH1106_DrawBitmap(right_arrow_img, 1, arrow_y, 8, 12, false);

	return screen;
}


void MENU_UpdateData(uint32_t _raw_brake_lever_angle, uint32_t _brake_lever_angle, uint32_t _servo_angle, uint32_t _batt_voltage_mv)
{
	raw_brake_angle = _raw_brake_lever_angle;
	brake_angle = _brake_lever_angle;
	s_angle = _servo_angle;
	battery_int_v = _batt_voltage_mv / 1000;
	battery_dec_v = _batt_voltage_mv / 10 - battery_int_v * 100;
}


void MENU_UpdateDebug(uint32_t kmh, uint32_t period, uint32_t last_capture_value, uint32_t last_capture_time, float current_freq, float avg_freq)
{
	debug_kmh = kmh;
	debug_period = period;
	debug_last_capture_value = last_capture_value;
	debug_last_capture_time = last_capture_time;
	debug_current_freq = current_freq;
	debug_avg_freq = avg_freq;
}
