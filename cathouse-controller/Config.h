#ifndef _CONFIG_H_
#define _CONFIG_H_

#define FIRMWARE_VER "cathouse-0.83"

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

// ease initial simulation of daily cat out there 8 hr
#define INITIAL_EMPTY_RUNTIME_HR 8

#define UPDATE_STATS_INTERVAL_MS 2000
#define UPDATE_TEMPERATURE_INTERVAL_MS 5000
#define FREERAM_THRESHOLD_MIN_BYTES (10*1024)
#define TEMPERATURE_HISTORY_BACKLOG_HOURS 48
#define ADCWEIGHT_HISTORY_BACKLOG_KB 4
#define ENGINE_POOL_INTERVAL_MS 5000
#define ADCWEIGHT_LASTSAMPLES_CNT 20

// port preference order
#define PORT_PREF_1 MOSFET_P2
#define PORT_PREF_2 MOSFET_P4
#define PORT_PREF_3 MOSFET_P1
#define PORT_PREF_4 MOSFET_P3

#define TBOTTOM_TREND_DELTA_C 0.5

#define FACTORY_TBOTTOM_LIMIT 40
#define FACTORY_TWOOD_LIMIT 50
#define FACTORY_TAMBIENT_LIMIT 17
#define FACTORY_COOLDOWN_TIME_MS (2*60*1000)
#define FACTORY_TEXTERN_GTE_SYS_OFF 14
#define FACTORY_ADC_WEIGHT_DELTA_CAT 18
#define FACTORY_TBOTTOM_GTE_FAN_ON 35
#define FACTORY_MANULA_MODE false
#define FACTORY_FANLESS_MODE true

// port {1,2,3,4,5,6} to pin
int portToPin(int pin);

#endif