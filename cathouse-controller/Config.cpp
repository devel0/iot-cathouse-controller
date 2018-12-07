#include "Config.h"
#include "EEUtil.h"

Config config;

const char *CSTR_firmwareVersion = "firmwareVersion";
const char *CSTR_wifiSSID = "wifiSSID";
const char *CSTR_temperatureHistoryFreeramThreshold = "temperatureHistoryFreeramThreshold";
const char *CSTR_temperatureHistoryBacklogHours = "temperatureHistoryBacklogHours";
const char *CSTR_updateConsumptionIntervalMs = "updateConsumptionIntervalMs";
const char *CSTR_updateFreeramIntervalMs = "updateFreeramIntervalMs";
const char *CSTR_updateTemperatureIntervalMs = "updateTemperatureIntervalMs";
const char *CSTR_tbottomLimit = "tbottomLimit";
const char *CSTR_twoodLimit = "twoodLimit";
const char *CSTR_tambientLimit = "tambientLimit";
const char *CSTR_cooldownTimeMs = "cooldownTimeMs";
const char *CSTR_tambientVsExternGTESysOff = "tambientVsExternGTESysOff";
const char *CSTR_tambientVsExternLTESysOn = "tambientVsExternLTESysOn";
const char *CSTR_tbottomGTEFanOn = "tbottomGTEFanOn";
const char *CSTR_tbottomLTEFanOff = "tbottomLTEFanOff";
const char *CSTR_autoactivateWoodBottomDeltaGTESysOn = "autoactivateWoodBottomDeltaGTESysOn";
const char *CSTR_autodeactivateWoodDeltaLT = "autodeactivateWoodDeltaLT";
const char *CSTR_autodeactivateInhibitAutoactivateMinMs = "autodeactivateInhibitAutoactivateMinMs";
const char *CSTR_autodeactivateExcursionSampleCount = "autodeactivateExcursionSampleCount";
const char *CSTR_autodeactivateExcursionSampleTotalMs = "autodeactivateExcursionSampleTotalMs";
const char *CSTR_texternGTESysOff = "texternGTESysOff";

void saveConfig()
{
    EEWrite(&config);
    Serial.println("config saved");
}

void loadConfig()
{
    EERead(&config);
    Serial.println("config loaded");
}

int heatPortIndexToPin(int idx)
{
    switch (idx)
    {
    case 0:
        return MOSFET_P1;
        break;
    case 1:
        return MOSFET_P2;
        break;
    case 2:
        return MOSFET_P3;
        break;
    case 3:
        return MOSFET_P4;
        break;
    }

    return MOSFET_P1;
}