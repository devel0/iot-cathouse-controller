#include "Config.h"

//--------------------------------------

#include <Arduino.h>

#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>

#include <EEPROM.h>
#include <OneWire.h>

#include "WiFiUtil.h"
#include "EEUtil.h"
#include "SerialOS.h"
#include "Util.h"
#include "TempDev.h"

void SetupPorts()
{
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

  while (Serial.available())
    Serial.read();

  EEInit();  

  SetupPorts();

  reconnectWifi();  

  SetupTemperatureDevices();
}

//=============================================================
// LOOP
//=============================================================

void loop()
{
  manageWifi();
  
  manageTemp();

  if (Serial.available())
  {
    auto c = (char)Serial.read();
    if ((int)c != 255)
    {
      if (c == '?' && serialInput.length() == 0)
        printSyntaxHelp();
      else if (c == '\r')
        processSerialCmd();
      else if (c == 8) // backspace
        serialInput.remove(serialInput.length() - 1);
      else
        serialInput.concat(c);
    }
  }

  if (server.status() == CLOSED)
    return;
}
