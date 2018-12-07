#include "EEUtil.h"

#include <EEPROM.h>

int EE_SIZE = sizeof(Config);

void EEInit()
{
    if (sizeof(Config) > EE_SIZE_MAX)
    {
        Serial.printf("EEData size %d > EESIZE_MAX %d\n", sizeof(Config), EE_SIZE_MAX);
        while (true)
            ;
    }
    EEPROM.begin(sizeof(Config));
    loadConfig();
    auto fwverlen = strnlen(config.firmwareVersion, CONFIG_FIRMWARE_STR_SIZE);
    // future: fw upgrade ( config.firmwareVersion != EEFD_firmwareVersion )
    if (fwverlen == CONFIG_FIRMWARE_STR_SIZE || strcmp(config.firmwareVersion, EEFD_firmwareVersion) != 0)
    {
        Serial.println("eeprom reset to factory defaults");
        EEResetToFactoryDefault(&config);
        saveConfig();
    }
}

void EEResetToFactoryDefault(Config *data)
{
    data->temperatureHistoryFreeramThreshold = EEFD_temperatureHistoryFreeramThreshold;
    data->temperatureHistoryBacklogHours = EEFD_temperatureHistoryBacklogHours;
    data->updateConsumptionIntervalMs = EEFD_updateConsumptionIntervalMs;
    data->updateFreeramIntervalMs = EEFD_updateFreeramIntervalMs;
    data->tbottomLimit = EEFD_tbottomLimit;
    data->twoodLimit = EEFD_twoodLimit;
    data->tambientLimit = EEFD_tambientLimit;
    data->cooldownTimeMs = EEFD_cooldownTimeMs;
    data->tambientVsExternGTESysOff = EEFD_tambientVsExternGTESysOff;
    data->tambientVsExternLTESysOn = EEFD_tambientVsExternLTESysOn;
    data->tbottomGTEFanOn = EEFD_tbottomGTEFanOn;
    data->tbottomLTEFanOff = EEFD_tbottomLTEFanOff;
    data->autoactivateWoodBottomDeltaGTESysOn = EEFD_autoactivateWoodBottomDeltaGTESysOn;
    data->autodeactivateWoodDeltaLT = EEFD_autodeactivateWoodDeltaLT;
    data->autodeactivateInhibitAutoactivateMinMs = EEFD_autodeactivateInhibitAutoactivateMinMs;
    data->autodeactivateExcursionSampleCount = EEFD_autodeactivateExcursionSampleCount;
    data->autodeactivateExcursionSampleTotalMs = EEFD_autodeactivateExcursionSampleTotalMs;
    data->texternGTESysOff = EEFD_texternGTESysOff;
}

void EERead(Config *data)
{
    auto ptr = (uint8_t *)data;
    for (int i = 0; i < EE_SIZE; ++i)
    {
        ptr[i] = EEPROM.read(i);
    }
}

void EEWrite(Config *data)
{
    auto ptr = (uint8_t *)data;
    for (int i = 0; i < EE_SIZE; ++i)
    {
        EEPROM.write(i, ptr[i]);
    }
    EEPROM.commit();
}
