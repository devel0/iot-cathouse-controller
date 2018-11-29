#ifndef _TEMP_DEV_H_
#define _TEMP_DEV_H_

#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>

extern int temperatureDeviceCount;
#define TEMPERATURE_ADDRESS_BYTES 8
extern char **tempDevHexAddress;
extern float *temperatures;
extern float **temperatureHistory;
extern uint8_t temperatureHistoryFillCnt;
extern uint16_t temperatureHistoryOff;

// computed : nr of sample foreach temperature device
extern uint16_t TEMPERATURE_HISTORY_SIZE;

// computed : interval of recording foreach temp device in order to span over desired TEMPERATURE_HISTORY_BACKLOG_HOURS
extern uint16_t TEMPERATURE_HISTORY_INTERVAL_SEC;

void SetupTemperatureDevices();
void ReadTemperatures();
void manageTemp();

#endif