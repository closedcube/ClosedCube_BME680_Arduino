/*

Arduino Library for Bosch Sensortec BME680 Environment Server
Written by AA for ClosedCube
---

The MIT License (MIT)

Copyright (c) 2017 ClosedCube Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/


#ifndef _CLOSEDCUBE_BME680_h

#define _CLOSEDCUBE_BME680_h
#include <Arduino.h>

#define BME680_REG_CTRL_HUM		0x72
#define BME680_REG_CTRL_MEAS	0x74
#define BME680_REG_CONFIG		0x75
#define BME680_REG_RESET		0xE0
#define BME680_REG_CHIPID		0xD0
#define BME680_REG_MEAS_STATUS	0x1D


enum ClosedCube_BME680_IIRFilter {
	BME680_FILTER_0		= 0x00,
	BME680_FILTER_1		= 0x01,
	BME680_FILTER_3		= 0x02,
	BME680_FILTER_7		= 0x03,
	BME680_FILTER_15	= 0x04,
	BME680_FILTER_31	= 0x05,
	BME680_FILTER_63	= 0x06,
	BME680_FILTER_127	= 0x07
};


enum ClosedCube_BME680_Oversampling {
	BME680_OVERSAMPLING_SKIP	= 0x00,
	BME680_OVERSAMPLING_X1		= 0x01,
	BME680_OVERSAMPLING_X2		= 0x02,
	BME680_OVERSAMPLING_X4		= 0x03,
	BME680_OVERSAMPLING_X8		= 0x04,
	BME680_OVERSAMPLING_X16		= 0x05
};


typedef union {
	uint8_t rawData;
	struct {
		uint8_t mode : 2;
		uint8_t osrs_p : 3;
		uint8_t osrs_t : 3;
	};
} ClosedCube_BME680_Ctrl_TP_Register;

typedef union {
	uint8_t rawData;
	struct {
		uint8_t osrs_h : 2;
		uint8_t unused : 6;
	};
} ClosedCube_BME680_Ctrl_H_Register;

typedef union {
	uint8_t rawData;
	struct {
		uint8_t gasMeasurementIndex : 3;
		uint8_t unused : 2;
		uint8_t measuringStatusFlag : 1;
		uint8_t gasMeasuringStatusFlag : 1;
		uint8_t newDataFlag : 2;
	};
} ClosedCube_BME680_Status;


struct bme680_cal_temp {
	uint16_t t1;
	uint16_t t2;
	uint8_t t3;
};

struct bme680_cal_pres {
	uint16_t p1;
	int16_t p2;
	int8_t p3;
	int16_t p4;
	int16_t p5;
	int8_t p6;
	int8_t p7;
	int16_t p8;
	int16_t p9;
	uint8_t p10;
};

struct bme680_cal_hum {
	uint16_t h1;
	uint16_t h2;
	int8_t h3;
	int8_t h4;
	int8_t h5;
	uint8_t h6;
	int8_t h7;
};

struct bme680_cal_dev {
	int32_t tfine;
};

class ClosedCube_BME680 {

public:
	ClosedCube_BME680();

	void init(uint8_t address);

	uint8_t getChipID();

	uint8_t reset();

	uint8_t setSleepMode();
	uint8_t setForcedMode();

	ClosedCube_BME680_Status readStatus();

	uint8_t setOversampling(ClosedCube_BME680_Oversampling humidity, ClosedCube_BME680_Oversampling temperature, ClosedCube_BME680_Oversampling pressure);
	

	float readTemperature();
	float readHumidity();
	float readPressure();

private:
	uint8_t _address;
	uint8_t _chipID;

	struct bme680_cal_temp _calib_temp;
	struct bme680_cal_pres _calib_pres;
	struct bme680_cal_hum _calib_hum;
	struct bme680_cal_dev _calib_dev;

	void loadCalData();

	uint8_t readByte(uint8_t cmd);
	uint8_t changeMode(uint8_t mode);
};


#endif 
