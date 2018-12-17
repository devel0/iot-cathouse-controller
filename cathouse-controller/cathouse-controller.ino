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
  Serial.println("Setup serial");
  Serial.begin(SERIAL_SPEED);
  //  Serial.swap(); // use D8(TX) - D7(RX)

  initEEStaticConfig();

  statsInit();

  setupPorts();

  // set led before wifi
  pinMode(LED_BUILTIN, OUTPUT);

  reconnectWifi();

  setupTemperatureDevices();
}

//=============================================================
// LOOP
//=============================================================

bool systemIsOn = false;

void loop()
{
  if (manageWifi())
    return;

  if (!serialOsActivated) statsUpdate();

  if (!serialOsActivated) manageTemp();  

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
}
