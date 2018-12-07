#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <Arduino.h>

#define SERIAL_SPEED 115200
#define WEB_PORT 80
#define ENABLE_CORS 1

#define ONEWIRE_PIN D3
#define BUILTIN_LED D4
#define LED_PIN D5
#define FAN_PIN D0

#define MOSFET_P1 D1
#define MOSFET_P2 D2
#define MOSFET_P3 D8
#define MOSFET_P4 D7

// measured system power / 4
#define W_PORT 26.75

int heatPortIndexToPin(int idx);

#define CONFIG_FIRMWARE_STR_SIZE 20
#define CONFIG_WIFI_SSID_STR_SIZE 30
#define CONFIG_WIFI_SSID_PWD_STR_SIZE 65

struct Config
{
    // firmware version
    char firmwareVersion[CONFIG_FIRMWARE_STR_SIZE];

    // wifi station id
    char wifiSSID[CONFIG_WIFI_SSID_STR_SIZE];

    // wifi password
    char wifiPwd[CONFIG_WIFI_SSID_PWD_STR_SIZE];

    // allocate temperature history so that it can hold temperatureHistoryBacklogHours hours
    // leaving temperatureHistoryFreeramThreshold free ram bytes
    unsigned int temperatureHistoryFreeramThreshold;

    // (see temperatureHistoryFreeramThreshold )
    unsigned int temperatureHistoryBacklogHours;

    // ms interval to update Wh consumption stat
    unsigned long updateConsumptionIntervalMs;

    // ms interval to update freeram stat
    unsigned long updateFreeramIntervalMs;

    // ms interval to update current temperature
    unsigned long updateTemperatureIntervalMs;

    // if bottom temp >= tbottomLimit heat ports gets disabled for cooldownTimeMs
    double tbottomLimit;

    // if wood temp >= twoodLimit heat ports gets disabled for cooldownTimeMs
    double twoodLimit;

    // if ambient temp >= tambientLimit heat ports gets disabled for cooldownTimeMs
    double tambientLimit;

    // heat ports disable time when cooldown condition occurs (see tbottomLimit, twoodLimit, tambientLimit)
    unsigned long cooldownTimeMs;

    // if tambient >= tambientVsExternGTESysOff heat ports enter disable state
    double tambientVsExternGTESysOff;

    // if tambient <= tambientVsExternLTESysOn heat ports enter enable state
    double tambientVsExternLTESysOn;

    // if tbottom >= tbottomGTEFanOn fan enter enable state
    double tbottomGTEFanOn;

    // if tbottom <= tbottomLTEFanOff fan enter disable state
    double tbottomLTEFanOff;

    // if (twood - tbottom) >= autoactivateWoodBottomDeltaGTESysOn system enter enable state
    double autoactivateWoodBottomDeltaGTESysOn;

    // if tbottom excursion over last autodeactivateExcursionSampleTotalMs interval <= autodeactivateWoodDeltaLT
    // system enter disable state for atleast autodeactivateInhibitAutoactivateMinMs timespan
    double autodeactivateWoodDeltaLT;

    // (see autodeactivateWoodDeltaLT)
    double autodeactivateInhibitAutoactivateMinMs;

    // (see also autodeactivateWoodDeltaLT)
    // stat past autodeactivateExcursionSampleTotalMs temperatures time interval using
    // a bunch of autodeactivateExcursionSampleCount (min,max) objects
    // when total stat interval exceeded oldest sample removed and added a new one on top (current time)
    int autodeactivateExcursionSampleCount;

    // (see autodeactivateExcursionSampleCount)
    // suggested value : at least 1.5 x heat cycle time interval
    unsigned long autodeactivateExcursionSampleTotalMs;

    // if textern >= texternGTESysOff system enter disable state
    double texternGTESysOff;
};

extern Config config;

void saveConfig();
void loadConfig();

//-----------
// EE FACTORY DEFAULTS
//
#define EEFD_firmwareVersion "cathouse-0.9"
#define EEFD_temperatureHistoryFreeramThreshold 10 * 1024
#define EEFD_temperatureHistoryBacklogHours 48
#define EEFD_updateConsumptionIntervalMs 2500
#define EEFD_updateFreeramIntervalMs 1000
#define EEFD_updateTemperatureIntervalMs 5000
#define EEFD_tbottomLimit 40
#define EEFD_twoodLimit 50
#define EEFD_tambientLimit 17
#define EEFD_cooldownTimeMs 2 * 60 * 1000
#define EEFD_tambientVsExternGTESysOff 11
#define EEFD_tambientVsExternLTESysOn 10
#define EEFD_tbottomGTEFanOn 30
#define EEFD_tbottomLTEFanOff 25
#define EEFD_autoactivateWoodBottomDeltaGTESysOn 3
#define EEFD_autodeactivateWoodDeltaLT 8.5
#define EEFD_autodeactivateInhibitAutoactivateMinMs 30 * 60 * 1000
#define EEFD_autodeactivateExcursionSampleCount 5
#define EEFD_autodeactivateExcursionSampleTotalMs 30 * 60 * 1000
#define EEFD_texternGTESysOff 14
//
//-----------

#endif