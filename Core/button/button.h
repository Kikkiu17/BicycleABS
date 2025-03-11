/*
 * button.h
 *
 *  Created on: Mar 3, 2025
 *      Author: Kikkiu
 */

#ifndef BUTTON_BUTTON_H_
#define BUTTON_BUTTON_H_

#include "stm32f1xx_hal.h"

typedef enum
{
  SHORT_PRESS = 0,
  LONG_PRESS = 1,
  NO_PRESS = 2,
} ButtonPress_Type;

#define SHORT_PRESS_TIME 160
#define LONG_PRESS_TIME 1000


uint8_t BUTTON_Handle(GPIO_TypeDef *GPIOx, uint16_t pin, uint8_t invert);


#endif /* BUTTON_BUTTON_H_ */
