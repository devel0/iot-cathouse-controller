#ifndef _EE_UTIL_H_
#define _EE_UTIL_H_

#define EE_WIFI_SSID_OFF 0
#define EE_WIFI_SSID_SIZE 64

#define EE_WIFI_PWD_OFF (EE_WIFI_SSID_OFF + EE_WIFI_SSID_SIZE)
#define EE_WIFI_PWD_SIZE 64

#define EE_SIZE 512

#include <Arduino.h>

void EEInit();

void readWifiSSID(String &res);
void readWifiPWD(String &res);
void writeWifiSSID(String str);
void writeWifiPWD(String str);

#endif