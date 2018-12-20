#ifndef _CONFIG_H_
#define _CONFIG_H_

#define FIRMWARE_VER "cathouse-0.82"

#include <Arduino.h>

#define SERIAL_SPEED 115200
#define WEB_PORT 80
#define ENABLE_CORS 1

#define ADCWEIGHT_PIN A0

#define ONEWIRE_PIN D3
#define LED_BUILTIN D4
#define LED_PIN D5
#define FAN_PIN D0

#define MOSFET_P1 D1
#define MOSFET_P2 D2
#define MOSFET_P3 D8
#define MOSFET_P4 D7

// measured system power / 4
#define W_PORT 26.75

int heatPortIndexToPin(int idx);

#define UPDATE_FREERAM_INTERVAL_MS 1000
#define UPDATE_CONSUMPTION_INTERVAL_MS 2500
#define UPDATE_TEMPERATURE_INTERVAL_MS 5000
#define UPDATE_ADCWEIGHT_INTERVAL_MS 2000
#define FREERAM_THRESHOLD_MIN_BYTES (10*1024)
#define TEMPERATURE_HISTORY_BACKLOG_HOURS 48
#define ADCWEIGHT_MEAN_SAMPLE_CNT 10

#endif