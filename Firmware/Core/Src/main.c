/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "As5600.h"
#include "servo.h"
#include "stm32f1xx_ll_exti.h"

#include "../sh1106/sh1106.h"
#include "../bitmaps/bitmaps.h"
#include "../menu/menu.h"
#include "../settings.h"
#include "../button/button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t kmh = 0;
int32_t last_kmh = 0;
int32_t kmh_diff = 0;
uint32_t brake_lever_angle = 0;
int32_t servo_angle = 0;
int32_t min_servo_angle = 0;

bool abs_on = false;				// if true, the ABS is currently operating
uint32_t abs_time = 0;			// time of the ABS activation

uint64_t apb1_timer_clock = 0;
uint64_t timer_update_constant = 0;
uint64_t period = 0;
uint32_t last_period = 0;
uint32_t last_capture_time = 0;

uint32_t light_mode = 0;

uint32_t last_capture_value = 0;
float last_freq = 0.00;
float current_freq = 0.00;
float avg_freq = 0.00;

char text[50];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
long mapInt(uint32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


uint32_t constrain(uint32_t value, uint32_t max, uint32_t min)
{
	if (value >= max)
		return max;
	else if (value <= min)
		return min;
	return value;
}


uint32_t offset(uint32_t value, uint32_t offset)
{
	if (value <= offset)
		return 0;
	else
		return value - offset;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);
  //HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_ADCEx_Calibration_Start(&hadc1);

  apb1_timer_clock = HAL_RCC_GetPCLK1Freq() * 2 / 100;
  timer_update_constant = (TIM3->PSC + 1) * (TIM3->ARR + 1) * (720 / ((TIM2->PSC + 1) / 1000));

  SERVO_Write(&htim1, TIM_CHANNEL_2, servo_max, 1);

  SH1106_Init();
  SH1106_SetBrightness(screen_brightness);
  SH1106_ClearScreen();

  AS5600_TypeDef as5600;
  AS5600_Init();
  Get_Magnet_Status(&as5600);
  bool magnet_status = as5600.Status_Magnet;

  if (magnet_status) SH1106_WriteChars(0, 0, "magnete ok", 10, Arial_12pt, 1);
  else
  {
	  SH1106_WriteChars(0, 0, "Magnete NON OK\nPremi un tasto\nper continuare", 44, Arial_12pt, 1);
	  while (1)
	  {
		  if (!HAL_GPIO_ReadPin(BTN_IN_GPIO_Port, BTN_IN_Pin))
			  break;
	  }
  }

  SH1106_ClearScreen();
  SH1106_SetScreenAutoUpdate(false);

  uint32_t flashing_lights_time = 0;
  uint32_t deltat = 0;
  uint32_t last_time = 0;
  float traveled_meters = 0.00;
  uint32_t mvbat = 0;
  bool show_low_bat = false;
  char decimal_travel_string[2];
  uint32_t raw_brake_lever_angle = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  deltat = uwTick - last_time;
	  last_time = uwTick;

	  // clear screen
	  SH1106_FillScreen(BLACK);

	  Get_Raw_Angle(&as5600);
	  raw_brake_lever_angle = as5600.Raw_sensor_Angle;
	  brake_lever_angle = offset(raw_brake_lever_angle, BRAKE_ANGLE_OFFSET);

	  if (!abs_on)
	  {
		  servo_angle = mapInt((int16_t)(brake_lever_angle), 0, BRAKE_ANGLE_MAX, servo_max, servo_min);
		  servo_angle = constrain(servo_angle, servo_max, servo_min);
	  }
	  else
	  {
		  // increase brake pressure gradually
		  if (servo_angle - SERVO_ABS_BRAKE_RATE > min_servo_angle && min_servo_angle >= servo_min)
			  servo_angle -= SERVO_ABS_BRAKE_RATE;
		  else abs_on = false;
	  }

	  if (mvbat > LOW_BATTERY_THRESHOLD)
	  {
		  if (servo_enabled)
			  SERVO_Write(&htim1, TIM_CHANNEL_2, servo_angle, 1);
		  else
		  {
			  if (!servo_hold)
				  SERVO_Write(&htim1, TIM_CHANNEL_2, servo_max, 1);
		  }
	  }
	  else
		  SERVO_Write(&htim1, TIM_CHANNEL_2, servo_max, 1);

	  // disable abs if brake lever is released
	  if (brake_lever_angle <= BRAKE_THRESHOLD || (kmh == 0 && brake_lever_angle <= BRAKE_THRESHOLD))
		  abs_on = false;

	  // -------------------- TRAVEL --------------------
	  traveled_meters += (float)kmh / 3.6 * (float)deltat / 1000.0;
	  uint32_t int_travel = traveled_meters / 1000;
	  uint32_t decimal_travel = traveled_meters - int_travel * 1000;

	  // -------------------- BATTERY VOLTAGE --------------------
	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	  uint32_t raw_value = HAL_ADC_GetValue(&hadc1);
	  float vbat = (float)raw_value / 4096.0 * 3.3 * 4;
	  mvbat = vbat * 1000;
	  uint32_t int_volt = vbat;
	  uint32_t decimal_volt = vbat * 100 - int_volt * 100;

	  // -------------------- HEADLIGHT AND BRAKE LIGHT --------------------
	  switch (light_mode)
	  {
	  case 0:
	  {
		  // -------------------- LIGHTS OFF --------------------
		  HAL_GPIO_WritePin(FRONT_LIGHTS_GPIO_Port, FRONT_LIGHTS_Pin, GPIO_PIN_RESET);

		  if (servo_angle <= BRAKE_THRESHOLD)
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, TIM1->ARR);
		  else
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

		  break;
	  }
	  case 1:
	  {
		  // -------------------- LIGHTS ON --------------------
		  HAL_GPIO_WritePin(FRONT_LIGHTS_GPIO_Port, FRONT_LIGHTS_Pin, GPIO_PIN_SET);

		  if (servo_angle <= BRAKE_THRESHOLD)
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, TIM1->ARR);
		  else
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, brake_light_pwm_period);

		  break;
	  }
	  case 2:
	  {
		  // -------------------- FLASHER --------------------
		  if (uwTick - flashing_lights_time > LIGHT_FLASHER_TIME)
		  {
			  flashing_lights_time = uwTick;

			  if (__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1) == TIM1->ARR)
				  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
			  else
				  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, TIM1->ARR);
		  }

		  break;
	  }
	  case 3:
	  {
		  light_mode = 0;
		  break;
	  }
	  }

	  // -------------------- DRAWING FUNCTIONS --------------------

	  if (mvbat > LOW_BATTERY_THRESHOLD)
	  {
		  // -------------------- BUTTON --------------------
		  uint8_t press_type = BUTTON_Handle(BTN_IN_GPIO_Port, BTN_IN_Pin, 1);

		  if (MENU_Update() != 0)
		  {
			  // -------------------- MENU --------------------
			  MENU_UpdateData(raw_brake_lever_angle, brake_lever_angle, servo_angle, mvbat);
			  MENU_UpdateDebug(kmh, period, last_capture_value, last_capture_time, current_freq, avg_freq);

			  if (press_type == SHORT_PRESS)
				  MENU_ButtonShortPress();
			  else if (press_type == LONG_PRESS)
				  MENU_ButtonLongPress();
			  SH1106_UpdateEntireFrame();
		  }
		  else
		  {
			  // -------------------- MAIN SCREEN --------------------
			  if (press_type == LONG_PRESS)
				  MENU_Set(1);
			  else if (press_type == SHORT_PRESS)
				  light_mode++;


			  // -------------------- LIGHT INDICATOR --------------------
			  if (light_mode == 1)
				  SH1106_DrawBitmap(light_img, 106, 12, 20, 15, false);
			  else if (light_mode == 2)
			  {
				  if (__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1) == TIM1->ARR)
					  SH1106_DrawBitmap(light_img, 106, 12, 20, 15, false);
			  }


			  // -------------------- SPEED --------------------
			  memset(text, 0, sizeof(text));
			  sprintf(text, "%" PRIu32, kmh);
			  uint32_t speed_width = SH1106_GetTextWidth(text, 2, Arial_48pt);
			  if (uwTick - abs_time < 2250)
			  {
				  sprintf(text, "%" PRIu32, kmh);
				  speed_width = SH1106_GetTextWidth(text, 3, Arial_48pt);
				  SH1106_WriteChars(64 - speed_width / 2, 7, text, 3, Arial_48pt, false);

				  SH1106_DrawBitmap(abs_img, 4, 34, 33, 25, false);
			  }
			  else
			  {
				  SH1106_WriteChars(64 - speed_width / 2, 7, text, 2, Arial_48pt, false);
			  }
			  sprintf(text, "kmh");
			  SH1106_WriteChars(68 + speed_width / 2, 38, text, 3, Arial_12pt, false);


			  // -------------------- TRAVEL --------------------
			  memset(text, 0, 6);
			  sprintf(text, "%" PRIu32, decimal_travel);
			  leftPadding(decimal_travel_string, text, 3, '0');
			  sprintf(text, "km:\n%" PRIu32 ".%s", int_travel, decimal_travel_string);
			  SH1106_WriteChars(4, 0, text, 8, Arial_12pt, false);


			  // -------------------- BATTERY --------------------
			  uint32_t battery_charge = mapInt(vbat * 1000, LOW_BATTERY_THRESHOLD, CHARGED_BATTERY_THRESHOLD, 0, BATTERY_CHARGE_WIDTH);
			  SH1106_DrawBitmap(battery_img, 112, 2, 15, 8, false);
			  SH1106_DrawRect(112, 2, battery_charge, 8, BLUE, false);
		  }

	  }
	  else
	  {
		  light_mode = 0;

		  if (uwTick - flashing_lights_time > LIGHT_FLASHER_TIME * 2)
		  {
			  flashing_lights_time = uwTick;

			  show_low_bat = !show_low_bat;
		  }

		  if (show_low_bat)
		  {
			  // LOW BATTERY
			  SH1106_DrawBitmap(battery_img, 108, 6, 15, 8, false);

			  memset(text, 0, sizeof(text));
			  sprintf(text, "BATTERIA\nSCARICA\n%" PRIu32 ".%" PRIu32 "V", int_volt, decimal_volt);
			  SH1106_WriteChars(6, 8, text, 22, Arial_12pt, false);
		  }
	  }

	  SH1106_UpdateEntireFrame();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	if (htim->Instance == TIM3)
	{
		// lock-up check cut-off is around 6 kmh
		if (kmh <= 6)
		{
			if (last_capture_time > uwTick) return;
			// 125 is a calibration value found during testing
			// 125 ms (calibrated; around 150 measured period) is the period
			// around which there is the transition
			// between 2 and 1 kmh (see kmh formula in callback below)
			if (uwTick - last_capture_time > 125)
				kmh = 0;
		}
		// if ((period (+ 3, optionally)) * apb1_timer_clock < timer_update_constant)
		// this is a simplified form of:
		// period * 1000 / 1200 < ((TIM3->PSC + 1) * (TIM3->ARR + 1)) * 1000 / apb1_timer_clock.
		// 3 is a calibration value found in testing to create a deadzone for the
		// lock-up detector. TIM3 is cleared every time an interrupt occurs
		else if ((period) * apb1_timer_clock < timer_update_constant)
		{
			kmh = 0;
			if (brake_lever_angle <= BRAKE_THRESHOLD)
			{
				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
				// lock-up
				if (abs_enabled)
					abs_on = true;		// true until the bike stops or no brake is applied
				abs_time = uwTick;

				min_servo_angle = servo_angle;
				servo_angle = servo_max;
				SERVO_Write(&htim1, TIM_CHANNEL_2, servo_angle, true);	// release the brake
			}
		}
	}
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	// capture compare is from TIM2
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3 && htim->Instance == TIM2)
	{
		uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
		// avoids weird numbers when TIM2 overflows
		if (abs((int)capture_value - (int)last_capture_value) > capture_value / 2)
		{
			last_capture_value = capture_value;
			return;
		}

		__HAL_TIM_SET_COUNTER(&htim3, 0);	// ABS timer reset

		if (last_capture_value > (last_period + capture_value))
			return;

		period = (last_period + capture_value - last_capture_value) / 2;

		last_capture_value = capture_value;
		last_period = period;
		last_capture_time = uwTick;

		// 1 / ((period * 30) / 1200)
		// there are 30 magnets in the encoder and TIM2 clocks at 1200 Hz

		// use this for smoother speed indication but less accurate
		// stop detection

		// (1.0 / 30.0 * (float)(720 / ((TIM2->PSC + 1) / 1000) * 1000))
		// is like 1 / 30 * (clock frequency)
		// if PSC = 15000: 1 / 30 * 4800 = 160

		current_freq = (1.0 / 30.0 * (float)(720 / ((TIM2->PSC + 1) / 1000) * 100)) / (float)period;
		avg_freq = (current_freq + last_freq) / 2.0;
		if (current_freq == 0)
			avg_freq = 0;
		//float freq = 40.0 / (float)period;

		// 2πrƒ = [m/s] * 3.6 = [km/h]
		kmh = 2 * 3.14 * avg_freq * 0.34 * 3.6;
		if (kmh > MAX_SPEED)
			kmh = 0;
		//if (kmh <= 3) kmh = 0;
		last_freq = avg_freq;
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
