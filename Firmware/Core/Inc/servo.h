/*
 * servo.h
 *
 *  Created on: Oct 19, 2023
 *      Author: Kikkiu
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "stm32f1xx_hal.h"

#define MAX_ANGLE 145

void SERVO_Write(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t angle, uint8_t inverted)
{
	uint32_t period = angle * 1500 / MAX_ANGLE + 1000;
	if (inverted)
		period = htim->Instance->ARR - period;
	__HAL_TIM_SET_COMPARE(htim, channel, period);
}


void SERVO_RAWWrite(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t percentage, uint8_t inverted)
{
	if (inverted)
		__HAL_TIM_SET_COMPARE(htim, channel, htim->Instance->ARR - htim->Instance->ARR * percentage / 100);
	else
		__HAL_TIM_SET_COMPARE(htim, channel, htim->Instance->ARR * percentage / 100);
}


#endif /* INC_SERVO_H_ */
