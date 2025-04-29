/*
 * aht10.h
 *
 *  Created on: May 14, 2024
 *      Author: Alperen Kopuz
 *      Github: @akopuz
 *
 *		based on:
 *      https://www.maritex.com.pl/product/attachment/147096/c5093eda6658ef654b651f3d5705c2ef
 *      https://github.com/adafruit/Adafruit_AHTX0
 */

#ifndef INC_AHT10_H_
#define INC_AHT10_H_

#include "i2c.h"

#define AHTX0_CALIBRATION_ENABLED

#define AHTX0_ADRESS		(0x38 << 1)	//sensor addr
#define AHTX0_CMD_INIT		0xE1		//initialization command
#define AHTX0_CMD_TRIGGER	0xAC		//trigger measurement
#define AHTX0_CMD_RESET		0xBA		//restart the sensor system

typedef enum {
	STATUS_BUSY 	= 0x80,
	STATUS_FREE		= 0x00,
	STATUS_ERROR	= 0xFF
} AHTX0_SensorState;

typedef enum {
	STATUS_CAL_ENABLE	= 0x08,
	STATUS_CAL_DISABLE	= 0x00,
	STATUS_CAL_ERROR	= 0xFF
} AHTX0_CalState;

typedef struct {
	I2C_HandleTypeDef * channel;
} AHTX0_InitTypeDef;

typedef struct {
	uint8_t buf[6];
	float temp;
	float hum;
} AHTX0_Data;


void ahtx0_begin(I2C_HandleTypeDef* );
AHTX0_SensorState ahtx0_get_status(void);
AHTX0_CalState ahtx0_get_cal_state(void);
void ahtx0_soft_reset(void);
void ahtx0_read_sensor(void);
float ahtx0_get_temperature(void);
float ahtx0_get_humidity(void);

extern AHTX0_InitTypeDef AHTX0_InitStruct;
extern AHTX0_Data AHTX0_DataStruct;

#endif /* INC_AHT10_H_ */
