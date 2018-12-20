#ifndef _TEMP_DEV_H_
#define _TEMP_DEV_H_

#include <Arduino.h>

#include <OneWire.h>
#include "BitArray.h"
#include <DallasTemperature.h>

extern int temperatureDeviceCount;
#define TEMPERATURE_ADDRESS_BYTES 8
extern char **tempDevHexAddress;
extern float *temperatures;
extern float **temperatureHistory;
extern uint16_t temperatureHistoryFillCnt;
extern uint16_t temperatureHistoryOff;

// computed : nr of sample foreach temperature device
extern uint16_t temperatureHistorySize;

// computed : interval of recording foreach temp device in order to span over desired TEMPERATURE_HISTORY_BACKLOG_HOURS
extern uint16_t temperatureHistoryIntervalSec;

extern BitArray *catInThereHistory;

void setupTemperatureDevices();
void readTemperatures();
void manageTemp();

extern float tbottom, twood, tambient, textern;

#endif