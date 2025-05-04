/*
 * As5600.c
 *
 *  Created on: Jul 10, 2024
 *      Author: EmrecanBl
 */

/**********************    INCLUDE DIRECTIVES    ***********************/

#include "as5600.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;
/**********************    GLOBAL VARIABLES    ***********************/

/*******************    FUNCTION IMPLEMENTATIONS    ********************/
void AS5600_Init(){
	uint8_t Data;
	Data = AS5600_POWER_MODE_DEFAULT;
	Data = Data|(AS5600_HYSTERESIS_DEFAULT<<2);
	Data = Data|(AS5600_OUTPUT_STAGE_DEFAULT<<4);
	Data = Data|(AS5600_FAST_FILTER_10LSB<<6);
	HAL_I2C_Mem_Write(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_CONF_HIGH, 1, &Data, 1, HAL_MAX_DELAY);
	Data = 0;
	Data = Data|(AS5600_FAST_FILTER_DEFAULT);
	Data = Data|(AS5600_SLOW_FILTER_DEFAULT<<2);
	Data = Data|(AS5600_WATCHDOG_DEFAULT<<4);
	HAL_I2C_Mem_Write(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_CONF_LOW, 1, &Data, 1, HAL_MAX_DELAY);
}

void AS5600_Set_Angle(AS5600_Config_TypeDef *Config){
	uint8_t Data;
	Data = Config->Burn_Angle;
	HAL_I2C_Mem_Write(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_ZMCO, 1, &Data, 1, 100);
	Data = Config->ZPOS;
	HAL_I2C_Mem_Write(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_ZPOS_HIGH, 1, &Data, 2, 100);
	Data = Config->MPOS;
	HAL_I2C_Mem_Write(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_MPOS_HIGH, 1, &Data, 2, 100);
	Data = Config->Maximum_angle;
	HAL_I2C_Mem_Write(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_MANG_HIGH, 1, &Data, 2, 100);
}
void Get_Magnet_Status(AS5600_TypeDef *Sensor){
	uint8_t Data[1];
	HAL_I2C_Mem_Read(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_STATUS, 1, &Data[1], 1, HAL_MAX_DELAY);
	Sensor->Status_Magnet = (Data[0]>>3)&(7);
}
void Get_AGC(AS5600_TypeDef *Sensor){
	uint8_t Data[1];
	HAL_I2C_Mem_Read(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_AGC, 1, &Data[1], 1, 100);
	Sensor->Value_AGC = Data[0];
}
void Get_MAGNITUDE(AS5600_TypeDef *Sensor){
	uint8_t Data[2];
	HAL_I2C_Mem_Read(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_MAGNITUDE_HIGH, 1, &Data[0], 1, 100);
	HAL_I2C_Mem_Read(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_MAGNITUDE_LOW, 1, &Data[1], 1, 100);
	Sensor->Value_Magnitude = Data[0]&(0<<4);
}
void Get_Raw_Angle(AS5600_TypeDef *Sensor){
	uint8_t Data[2];
	HAL_I2C_Mem_Read(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_RAW_ANGLE_HIGH, 1, &Data[0], 1, 100);
	HAL_I2C_Mem_Read(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_RAW_ANGLE_LOW, 1, &Data[1], 1, 100);
	Sensor->Raw_Data = (int16_t)(Data[0]<<8)|(Data[1]);
	Sensor->Raw_sensor_Angle = (int)(Sensor->Raw_Data*360/4096);
}

void Get_Angle(AS5600_TypeDef *Sensor){
	uint8_t Data[2];
	HAL_I2C_Mem_Read(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_ANGLE_HIGH, 1, &Data[0], 1, 100);
	HAL_I2C_Mem_Read(&hi2c1, AS5600_SLAVE_ADDRESS , AS5600_REGISTER_ANGLE_LOW, 1, &Data[1], 1, 100);
	Sensor->Raw_Data = (int16_t)(Data[0]<<8)|(Data[1]);
	Sensor->sensor_Angle = (int)(Sensor->Raw_Data*360/4096);
	readings[idx] = Sensor->sensor_Angle;
	idx = (idx + 1) % MEDIAN_FILTER_SIZE;
	uint16_t median = getMedian(readings, MEDIAN_FILTER_SIZE);
	uint16_t newReading = median;
	sum -= readings_m[index_m];
	readings_m[index_m] = newReading;
	sum += newReading;
    index_m = (index_m + 1) % MOVING_AVERAGE_SIZE;
    if (count < MOVING_AVERAGE_SIZE) {
            count++;
        }
    uint16_t average = sum / count;
    Sensor->Filtered_Angle = average;

}

void medianFilter(uint16_t *data, uint8_t size) {
    for (uint8_t i = 0; i < size - 1; i++) {
        for (uint8_t j = 0; j < size - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                uint16_t temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}
uint16_t getMedian(uint16_t *data, uint8_t size) {
    medianFilter(data, size);
    if (size % 2 == 0) {
        return (data[size / 2 - 1] + data[size / 2]) / 2;
    } else {
        return data[size / 2];
    }
}
