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

#define UPDATE_STATS_INTERVAL_MS 2000
#define UPDATE_TEMPERATURE_INTERVAL_MS 5000
#define FREERAM_THRESHOLD_MIN_BYTES (10*1024)
#define TEMPERATURE_HISTORY_BACKLOG_HOURS 48
#define ADCWEIGHT_HISTORY_BACKLOG_KB 4
#define ENGINE_POOL_INTERVAL_MS 5000
#define ADCWEIGHT_LASTSAMPLES_CNT 20

#define FACTORY_TBOTTOM_LIMIT 40
#define FACTORY_TWOOD_LIMIT 50
#define FACTORY_TAMBIENT_LIMIT 17
#define FACTORY_COOLDOWN_TIME_MS (2*60*1000)
#define FACTORY_STANDBY_DURATION_MS (30 * 60 * 1000)
#define FACTORY_STANDBY_PORT 2
#define FACTORY_FULLPOWER_DURATION_MS (20 * 60 * 1000)
#define FACTORY_TEXTERN_GTE_SYS_OFF 14
#define FACTORY_ADC_WEIGHT_DELTA_CAT 18
#define FACTORY_TBOTTOM_GTE_FAN_ON 20
#define FACTORY_MANULA_MODE false
#define FACTORY_FANLESS_MODE true
#define FACTORY_PORT_DURATION_MS (7 * 60 * 1000)
#define FACTORY_PORT_OVERLAP_DURATION_MS (2 * 60 * 1000)

// port {1,2,3,4,5,6} to pin
int portToPin(int pin);

#endif