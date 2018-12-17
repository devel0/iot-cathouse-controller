#ifndef _WIFI_UTIL_H_
#define _WIFI_UTIL_H_

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiServer.h>

enum CCTypes { HTML, JSON, TEXT, JAVASCRIPT, PNG };

extern WiFiServer server;
extern String header;

void reconnectWifi();
void clientEnd(WiFiClient &client);
void clientOk(WiFiClient &client, CCTypes type);
void clientWriteBigString(WiFiClient &client, const __FlashStringHelper *str);
void clientWriteBinary(WiFiClient &client, unsigned char arr[], unsigned int l);
void clientWriteBinaryF(WiFiClient &client, const unsigned char arr[], unsigned int l);
void printWifiData();
void printCurrentNet();

#endif