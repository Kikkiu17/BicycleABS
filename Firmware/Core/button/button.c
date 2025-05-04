/*
 * button.c
 *
 *  Created on: Mar 3, 2025
 *      Author: Kikkiu
 */

#include "button.h"

uint8_t button_pressed = 0;
uint32_t button_press_time = INT32_MAX;


uint8_t BUTTON_Handle(GPIO_TypeDef *GPIOx, uint16_t pin, uint8_t invert)
{
	if (invert)
	{
		if (!HAL_GPIO_ReadPin(GPIOx, pin))
		{
			if (!button_pressed)
			{
			  button_pressed = 1;
			  button_press_time = uwTick;
			}
		}
		else
		{
			button_pressed = 0;

			if (uwTick - button_press_time < SHORT_PRESS_TIME)
			{
				button_press_time = INT32_MAX;
				return SHORT_PRESS;
			}
			else if (uwTick - button_press_time < LONG_PRESS_TIME)
			{
				button_press_time = INT32_MAX;
				return LONG_PRESS;
			}

			button_press_time = INT32_MAX;
		}
	}

	return NO_PRESS;
}

