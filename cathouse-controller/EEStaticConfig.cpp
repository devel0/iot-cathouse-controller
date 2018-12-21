#include "EEStaticConfig.h"

#include "Config.h"
#include "EEJsonConfig.h"

#include <EEPROM.h>

EEStaticConfig eeStaticConfig;
bool eeStaticConfigDirty = false;

void stringSanityCheck()
{
    // preserve ssid,pwd,temp ids contents but ensure string termination
    eeStaticConfig.wifiSSID[WIFI_SSID_STRLENMAX] = 0;
    eeStaticConfig.wifiPwd[WIFI_PWD_STRLENMAX] = 0;
    eeStaticConfig.tbottomId[DS18B20_ID_STRLENMAX] = 0;
    eeStaticConfig.twoodId[DS18B20_ID_STRLENMAX] = 0;
    eeStaticConfig.tambientId[DS18B20_ID_STRLENMAX] = 0;
    eeStaticConfig.texternId[DS18B20_ID_STRLENMAX] = 0;

    // ensure no doublequotes
    if (strchr(eeStaticConfig.wifiSSID, (char)'"') != NULL)
        eeStaticConfig.wifiSSID[0] = 0;
    if (strchr(eeStaticConfig.wifiPwd, (char)'"') != NULL)
        eeStaticConfig.wifiPwd[0] = 0;
    if (strchr(eeStaticConfig.tbottomId, (char)'"') != NULL)
        eeStaticConfig.tbottomId[0] = 0;
    if (strchr(eeStaticConfig.twoodId, (char)'"') != NULL)
        eeStaticConfig.twoodId[0] = 0;
    if (strchr(eeStaticConfig.tambientId, (char)'"') != NULL)
        eeStaticConfig.tambientId[0] = 0;
    if (strchr(eeStaticConfig.texternId, (char)'"') != NULL)
        eeStaticConfig.texternId[0] = 0;
}

void initEEStaticConfig()
{
    EEPROM.begin(EE_SIZE);

    loadEEStaticConfig();
    auto fwverlen = strnlen(eeStaticConfig.firmwareVersion, FIRMWARE_VERSION_STRLENMAX + 1);
    if (fwverlen == (FIRMWARE_VERSION_STRLENMAX + 1) || strcmp(eeStaticConfig.firmwareVersion, FIRMWARE_VER) != 0)
    {
        Serial.println("firmware version changed -> eeprom reset to factory defaults");
        strcpy(eeStaticConfig.firmwareVersion, FIRMWARE_VER);

        saveEEStaticConfig();

        // zeroes json config
        Serial.println("clearing eeJsonConfig");
        eeJsonConfig.Clear();
        eeJsonConfig.SaveToEEProm();
    }
}

void loadEEStaticConfig()
{
    auto ptr = (uint8_t *)&eeStaticConfig;
    auto l = sizeof(EEStaticConfig);
    for (int i = 0; i < l; ++i)
        ptr[i] = EEPROM.read(i);
    stringSanityCheck();
}

void saveEEStaticConfig()
{
    auto ptr = (uint8_t *)&eeStaticConfig;
    auto l = sizeof(EEStaticConfig);
    for (int i = 0; i < l; ++i)
        EEPROM.write(i, ptr[i]);
    EEPROM.commit();
}
