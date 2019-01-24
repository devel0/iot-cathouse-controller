#include "Config.h"

//--------------------------------------

#include <Arduino.h>

#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>

#include <EEPROM.h>
#include <OneWire.h>

#include "WiFiUtil.h"
#include "WiFiMain.h"
#include "EEJsonConfig.h"
#include "SerialOS.h"
#include "Util.h"
#include "TempDev.h"
#include "WeightDev.h"
#include "Stats.h"
#include "Engine.h"

void setupPorts()
{
  pinMode(A0, INPUT);

  pinMode(MOSFET_P1, OUTPUT);
  digitalWrite(MOSFET_P1, LOW);

  pinMode(MOSFET_P2, OUTPUT);
  digitalWrite(MOSFET_P2, LOW);

  pinMode(MOSFET_P3, OUTPUT);
  digitalWrite(MOSFET_P3, LOW);

  pinMode(MOSFET_P4, OUTPUT);
  digitalWrite(MOSFET_P4, LOW);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
}

//=============================================================
// SETUP
//=============================================================

void setup()
{
  Serial.begin(SERIAL_SPEED);
  //  Serial.swap(); // use D8(TX) - D7(RX)

  Serial.printf("Initial delay...\n");
  {
    //long m = millis();
    //while (timeDiff(m, millis()) < 5000) ;
    delay(5000);
  }

  Serial.printf("STARTUP\n");

  initEEStaticConfig();

  eeJsonConfig.LoadFromEEProm();

  statsInit();

  setupPorts();

  setupTemperatureDevices();

  // set led before wifi
  pinMode(LED_BUILTIN, OUTPUT);

  reconnectWifi();

  Serial.printf("SETUP DONE\n");
}

//=============================================================
// LOOP
//=============================================================

bool systemIsOn = false;

// https://github.com/esp8266/Arduino/issues/4161
boolean WiFiReturns()
{
  if (WiFi.localIP() == IPAddress(0, 0, 0, 0))
    return 0;
  switch (WiFi.status())
  {
  case WL_NO_SHIELD:
    return 0;
  case WL_IDLE_STATUS:
    return 0;
  case WL_NO_SSID_AVAIL:
    return 0;
  case WL_SCAN_COMPLETED:
    return 1;
  case WL_CONNECTED:
    return 1;
  case WL_CONNECT_FAILED:
    return 0;
  case WL_CONNECTION_LOST:
    return 0;
  case WL_DISCONNECTED:
    return 0;
  default:
    return 0;
  }
}

auto tloop = millis();

auto wifiReconnecting = false;

void loop()
{
  {
    auto t = millis();
    freeram = freeMemorySum();
    freeram_min = min(freeram, freeram_min);
    auto tdiff = timeDiff(t, millis());
    if (tdiff > 500)
      Serial.printf("freeram_min took %ld\n", tdiff);
  }
  
  if (!WiFiReturns())
  {
    if (!wifiReconnecting)
    {
      Serial.printf("wifi reconnecting\n");
      wifiReconnecting = true;
    }
    reconnectWifi();
    return;
  }

  wifiReconnecting = false;

  {
    auto t = millis();
    auto res = manageWifi();
    auto tdiff = timeDiff(t, millis());
    if (tdiff > 500)
      Serial.printf("manageWifi took %ld\n", tdiff);
    if (res)
      return;
  }

/*
  auto m = millis();
  if (timeDiff(tloop, m) > 5000)
  {
    Serial.printf("tloop[%ld]\n", m);
    tloop = m;
  }*/    

  if (!serialOsActivated)
  {
    auto t = millis();
    statsUpdate();
    auto tdiff = timeDiff(t, millis());
    if (tdiff > 500)
      Serial.printf("statsUpdate took %ld\n", tdiff);
  }

  if (!serialOsActivated)
  {
    auto t = millis();
    manageTemp();
    auto tdiff = timeDiff(t, millis());
    if (tdiff > 500)
      Serial.printf("manageTemp took %ld\n", tdiff);
  }

  if (!serialOsActivated)
  {
    auto t = millis();
    engineProcess();
    auto tdiff = timeDiff(t, millis());
    if (tdiff > 500)
      Serial.printf("engineProcess took %ld\n", tdiff);
  }

  {
    auto t = millis();
    if (Serial.available())
    {
      builtinLedOff();

      auto c = (char)Serial.read();
      if ((int)c != 255)
      {
        if (c == '?' && serialInput.length() == 0)
        {
          printSyntaxHelp();
          serialOsActivated = true;
        }
        else if (c == '\r')
          processSerialCmd();
        else if (c == 8) // backspace
          serialInput.remove(serialInput.length() - 1);
        else
          serialInput.concat(c);
      }

      builtinLedOn();
    }
    auto tdiff = timeDiff(t, millis());
    if (tdiff > 500)
      Serial.printf("serialOs took %ld\n", tdiff);
  }
}
