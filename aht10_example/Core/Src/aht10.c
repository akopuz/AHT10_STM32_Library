/*
 * aht10.c
 *
 *  Created on: May 14, 2024
 *      Author: Alperen Kopuz
 */

#include "aht10.h"

AHTX0_InitTypeDef AHTX0_InitStruct = {0};
AHTX0_Data AHTX0_DataStruct = {0};

/**
 * @brief  Initializes the AHTX0 temperature and humidity sensor.
 *         Sets the I2C communication channel and performs a software reset.
 *         If calibration is enabled, it sends a calibration command to the sensor
 *         and waits until the calibration process is complete.
 *
 * @param  hi2c: Pointer to the I2C handle to be used for communication (I2C_HandleTypeDef)
 */
void ahtx0_begin(I2C_HandleTypeDef* hi2c)
{
	AHTX0_InitStruct.channel = hi2c;
	ahtx0_soft_reset();
	HAL_Delay(100);

#ifdef	AHTX0_CALIBRATION_ENABLED
	uint8_t calibrate_buf[3] = {AHTX0_CMD_TRIGGER, STATUS_CAL_ENABLE, 0x00};
	HAL_I2C_Master_Transmit(hi2c, AHTX0_ADRESS, calibrate_buf, 3, 100);
	while(!ahtx0_get_cal_state());
#endif

}

/**
 * @brief  Reads the current status of the AHTX0 sensor.
 *         Communicates via I2C to receive the sensor's status byte.
 *
 * @retval AHTX0_SensorState:
 *         - STATUS_ERROR if an error is detected.
 *         - STATUS_FREE if the sensor is ready for communication.
 *         - STATUS_BUSY if the sensor is currently busy.
 */
AHTX0_SensorState ahtx0_get_status(void)
{
	uint8_t ret = 0x00;
	HAL_I2C_Master_Receive(AHTX0_InitStruct.channel, AHTX0_ADRESS, &ret, 1, 100);

	if(ret == STATUS_ERROR)
	{
		return ret;
	}
	else if(!(ret & STATUS_BUSY))
	{
		return STATUS_FREE;
	}

	return STATUS_BUSY;
}

/**
 * @brief  Checks the calibration status of the AHTX0 sensor.
 *         Reads the status byte via I2C and determines whether the sensor is calibrated.
 *
 * @retval AHTX0_CalState:
 *         - STATUS_CAL_ERROR if an error is detected.
 *         - STATUS_CAL_DISABLE if the sensor is not calibrated.
 *         - STATUS_CAL_ENABLE if the sensor is successfully calibrated.
 */
AHTX0_CalState ahtx0_get_cal_state(void)
{
	uint8_t ret = 0x00;
	HAL_I2C_Master_Receive(AHTX0_InitStruct.channel, AHTX0_ADRESS, &ret, 1, 100);

	if(ret == STATUS_CAL_ERROR)
	{
		return ret;
	}
	else if(!(ret & STATUS_CAL_ENABLE))
	{
		return STATUS_CAL_DISABLE;
	}

	return STATUS_CAL_ENABLE;
}

/**
 * @brief  This command is used to restart the
 *		sensor system without turning the power off and on
 *		again. After receiving this command, the sensor
 *		system begins to reinitialize and restores the default
 *		settings.
 *
 */
void ahtx0_soft_reset(void)
{
	uint8_t temp = AHTX0_CMD_RESET;
	HAL_I2C_Master_Transmit(AHTX0_InitStruct.channel, AHTX0_ADRESS, &temp, 1, 100);
}

/**
 * @brief  Triggers a measurement and reads temperature and humidity data from the AHT10 sensor.
 *         Sends a trigger command via I2C, waits until the sensor is ready,
 *         then reads 6 bytes of data and calculates temperature and humidity values.
 *
 *         Temperature is calculated using bits [19:0] starting from byte 3 (low nibble), 4, and 5.
 *         Humidity is calculated using bits [19:0] starting from byte 1, 2, and high nibble of byte 3.
 */
void ahtx0_read_sensor(void)
{
	uint8_t trigger_buf[3] = {AHTX0_CMD_TRIGGER, 0x33, 0x00};
	HAL_I2C_Master_Transmit(AHTX0_InitStruct.channel, AHTX0_ADRESS, trigger_buf, 3, 100);

	while(ahtx0_get_status());

	HAL_I2C_Master_Receive(AHTX0_InitStruct.channel, AHTX0_ADRESS, AHTX0_DataStruct.buf, 6, 100);

	uint32_t temp = (((uint32_t)AHTX0_DataStruct.buf[3] & 15) << 16) | ((uint32_t)AHTX0_DataStruct.buf[4] << 8) | AHTX0_DataStruct.buf[5];
	AHTX0_DataStruct.temp = (float)(temp * 200.0f / 1048576.0f) - 50.0f;

	temp = ((uint32_t)AHTX0_DataStruct.buf[1] << 12) | ((uint32_t)AHTX0_DataStruct.buf[2] << 4) | (AHTX0_DataStruct.buf[3] >> 4);
	AHTX0_DataStruct.hum = (float)(temp*100.0f/1048576.0f);

}
/**
 * @brief  Returns the last measured temperature value.
 *
 * @retval float: Temperature in degrees Celsius.
 */
float ahtx0_get_temperature(void)
{
	return AHTX0_DataStruct.temp;
}

/**
 * @brief  Returns the last measured humidity value.
 *
 * @retval float: Relative humidity in percentage (%RH).
 */
float ahtx0_get_humidity(void)
{
	return AHTX0_DataStruct.hum;
}



