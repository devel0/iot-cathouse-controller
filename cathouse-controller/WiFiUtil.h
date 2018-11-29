#ifndef _WIFI_UTIL_H_
#define _WIFI_UTIL_H_

#include <Arduino.h>

#include "Config.h"
#include <ESP8266WiFi.h>
#include <WiFiServer.h>

extern WiFiServer server;

void reconnectWifi();
void manageWifi();

void printWifiData();
void printCurrentNet();

#endif