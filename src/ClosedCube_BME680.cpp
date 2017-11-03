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


#include <Wire.h>
#include "ClosedCube_BME680.h"


ClosedCube_BME680::ClosedCube_BME680() {
}

void ClosedCube_BME680::init(uint8_t address) {
	_address = address;
	_chipID = readByte(BME680_REG_CHIPID);

	loadCalData();
}

uint8_t ClosedCube_BME680::reset() {
	return readByte(BME680_REG_RESET);
}

uint8_t ClosedCube_BME680::getChipID() {
	return _chipID;
}

uint8_t ClosedCube_BME680::setSleepMode() {
	return changeMode(0x00);
}

uint8_t ClosedCube_BME680::setForcedMode() {
	return changeMode(0x01);
}

uint8_t ClosedCube_BME680::changeMode(uint8_t mode) {
	ClosedCube_BME680_Ctrl_TP_Register ctrl_meas;

	ctrl_meas.rawData = readByte(BME680_REG_CTRL_MEAS);
	ctrl_meas.mode = mode;

	Wire.beginTransmission(_address);
	Wire.write(BME680_REG_CTRL_MEAS);
	Wire.write(ctrl_meas.rawData);
	return Wire.endTransmission();
}

ClosedCube_BME680_Status ClosedCube_BME680::readStatus() {
	ClosedCube_BME680_Status status;
	status.rawData = readByte(BME680_REG_MEAS_STATUS);
	return status;
}

float ClosedCube_BME680::readHumidity() {
	uint8_t hum_msb = readByte(0x25);
	uint8_t hum_lsb = readByte(0x26);

	uint16_t hum_raw = hum_msb << 8 | hum_lsb;

	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t var4;
	int32_t var5;
	int32_t var6;
	int32_t temp;

	temp = ((_calib_dev.tfine * 5) + 128) / 256;
	var1 = (hum_raw - _calib_hum.h1 * 16) - ((temp *_calib_hum.h3) / 100 / 2);
	var2 = (_calib_hum.h2 * ((temp * _calib_hum.h4 / 100)
			+ (((temp * (temp * _calib_hum.h5 / 100)) / 64) / 100) + (1 * 16384))) / 1024;
	var3 = var1 * var2;
	var4 = _calib_hum.h6 * 128;
	var4 = ((var4)+((temp * _calib_hum.h7) / 100)) / 16;
	var5 = ((var3 / 16384) * (var3 / 16384)) / 1024;
	var6 = (var4 * var5) / 2;

	return (var3 + var6) / 1024 / 4096.0f;
}

float ClosedCube_BME680::readPressure() {
	uint8_t pres_msb = readByte(0x1F);
	uint8_t pres_lsb = readByte(0x20);
	uint8_t pres_xlsb = readByte(0x21);

	uint32_t pres_raw = ((uint32_t)pres_msb << 12) | ((uint32_t)pres_lsb << 4) | ((uint32_t)pres_xlsb >> 4);

	int32_t var1;
	int32_t var2;
	int32_t var3;

	var1 = _calib_dev.tfine / 2 - 64000;
	var2 = ((var1 / 4) * (var1 / 4)) / 2048;
	var2 = (var2 * _calib_pres.p6) / 4;
	var2 = var2 + ((var1 * _calib_pres.p5) * 2);
	var2 = (var2 / 4) + (_calib_pres.p4 * 65536);
	var1 = ((var1 / 4) * (var1 / 4)) / 8192;
	var1 = (((var1) * _calib_pres.p3 * 32) / 8) + ((_calib_pres.p2 * var1) / 2);
	var1 = var1 / 262144;
	var1 = ((32768 + var1) * _calib_pres.p1) / 32768;
	pres_raw = 1048576 - pres_raw;
	pres_raw = (pres_raw - (var2 / 4096)) * (3125);
	pres_raw = ((pres_raw / var1) * 2);
	var1 = ((int32_t)_calib_pres.p9 * (int32_t)(((pres_raw / 8) * (pres_raw / 8)) / 8192)) / 4096;
	var2 = ((int32_t)(pres_raw / 4) * (int32_t)_calib_pres.p8) / 8192;
	var3 = ((pres_raw / 256) * (pres_raw / 256) * (pres_raw / 256) * _calib_pres.p10) / 131072;
	pres_raw = pres_raw+((var1 + var2 + var3 + (_calib_pres.p7 * 128)) / 16);

	return pres_raw / 100.0f;
}

float ClosedCube_BME680::readTemperature() {
	uint8_t temp_msb = readByte(0x22);
	uint8_t temp_lsb = readByte(0x23);
	uint8_t temp_xlsb = readByte(0x24);

	uint32_t temp_raw = ((uint32_t)temp_msb << 12) | ((uint32_t)temp_lsb << 4) | ((uint32_t)temp_xlsb >> 4);

	uint32_t var1;
	uint32_t var2;
	uint32_t var3;

	var1 = (temp_raw / 8) - (_calib_temp.t1 * 2);
	var2 = (var1 * _calib_temp.t2) / 2048;
	var3 = ((var1 / 2) * (var1 / 2)) / 4096;
	var3 = ((var3) * (_calib_temp.t3 * 16)) / 16384;
	_calib_dev.tfine = var2 + var3;
	return (((_calib_dev.tfine * 5) + 128) / 256) / 100.0f;
}

uint8_t ClosedCube_BME680::setOversampling(ClosedCube_BME680_Oversampling humidity, ClosedCube_BME680_Oversampling temperature, ClosedCube_BME680_Oversampling pressure) {	
	ClosedCube_BME680_Ctrl_TP_Register ctrl_meas;
	ctrl_meas.osrs_t = temperature;
	ctrl_meas.osrs_p = pressure;
	ctrl_meas.mode = 0x0;

	Wire.beginTransmission(_address);
	Wire.write(BME680_REG_CTRL_HUM);
	Wire.write(humidity);
	Wire.write(BME680_REG_CTRL_MEAS);
	Wire.write(ctrl_meas.rawData);
	return Wire.endTransmission();
}

uint8_t ClosedCube_BME680::readByte(uint8_t cmd) {
	Wire.beginTransmission(_address);
	Wire.write(cmd);
	Wire.endTransmission();

	Wire.requestFrom(_address, (uint8_t)1);
	Wire.read();
}

void ClosedCube_BME680::loadCalData() {
	uint8_t cal1[25];
	uint8_t cal2[16];

	Wire.beginTransmission(_address);
	Wire.write(0x89);
	Wire.endTransmission();

	Wire.requestFrom(_address, (uint8_t)25);
	Wire.readBytes(cal1, 25);

	Wire.beginTransmission(_address);
	Wire.write(0xE1);
	Wire.endTransmission();

	Wire.requestFrom(_address, (uint8_t)16);
	Wire.readBytes(cal2, 16);

	_calib_temp.t1 = cal2[9] << 8 | cal2[8];
	_calib_temp.t2 = cal1[2] << 8 | cal1[1];
	_calib_temp.t3 = cal1[3];
	
	_calib_hum.h1 = cal2[2] << 4 | (cal2[1] & 0x0F);
	_calib_hum.h2 = cal2[0] << 4 | cal2[1];
	_calib_hum.h3 = cal2[3];
	_calib_hum.h4 = cal2[4];
	_calib_hum.h5 = cal2[5];
	_calib_hum.h6 = cal2[6];
	_calib_hum.h7 = cal2[7];

	_calib_pres.p1 = cal1[6] << 8 | cal1[5];
	_calib_pres.p2 = cal1[8] << 8 | cal1[7];
	_calib_pres.p3 = cal1[9];
	_calib_pres.p4 = cal1[12] << 8 | cal1[11];
	_calib_pres.p5 = cal1[14] << 8 | cal1[13];
	_calib_pres.p6 = cal1[16];
	_calib_pres.p7 = cal1[15];
	_calib_pres.p8 = cal1[20] << 8 | cal1[19];
	_calib_pres.p9 = cal1[22] << 8 | cal1[21];
	_calib_pres.p10 = cal1[23];


}