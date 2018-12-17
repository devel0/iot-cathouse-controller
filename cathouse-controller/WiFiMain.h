#ifndef _WIFI_MAIN_H_
#define _WIFI_MAIN_H_

#include <Arduino.h>

#include "Config.h"
#include <ESP8266WiFi.h>
#include <WiFiServer.h>

extern WiFiServer server;

void reconnectWifi();
bool manageWifi();

#endif