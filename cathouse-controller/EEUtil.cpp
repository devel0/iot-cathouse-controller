#include "EEUtil.h"

#include <EEPROM.h>

void EEInit()
{
    EEPROM.begin(EE_SIZE);
}

void readWifiSSID(String &res)
{
    res = "";
    for (int i = EE_WIFI_SSID_OFF; i < EE_WIFI_SSID_OFF + EE_WIFI_SSID_SIZE; ++i)
    {
        auto c = EEPROM.read(i);
        if (c == 0)
            break;
        res.concat((char)c);
    }
    Serial.printf("readed ssid [%s] from eeprom\n", res.c_str());
}

void readWifiPWD(String &res)
{
    res = "";
    for (int i = EE_WIFI_PWD_OFF; i < EE_WIFI_PWD_OFF + EE_WIFI_PWD_SIZE; ++i)
    {
        auto c = EEPROM.read(i);
        if (c == 0)
            break;
        res.concat((char)c);
    }
}

void writeWifiSSID(String str)
{
    auto l = str.length();
    int j = 0;
    int i = EE_WIFI_SSID_OFF;
    while (j < l && i < EE_WIFI_SSID_OFF + EE_WIFI_SSID_SIZE)
    {
        auto c = (uint8_t)str[j++];
        EEPROM.write(i++, c);
    }
    EEPROM.write(i, 0);
    EEPROM.commit();

    Serial.printf("saved ssid [%s] to eeprom\n", str.c_str());
}

void writeWifiPWD(String str)
{
    auto l = str.length();
    int j = 0;
    int i = EE_WIFI_PWD_OFF;
    while (j < l && i < EE_WIFI_PWD_OFF + EE_WIFI_PWD_SIZE)
    {
        auto c = (uint8_t)str[j++];
        EEPROM.write(i++, c);
    }
    EEPROM.write(i, 0);
    EEPROM.commit();

    Serial.printf("saved pwd to eeprom\n");
}