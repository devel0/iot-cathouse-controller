#include "SerialOS.h"

#include <Arduino.h>

#include "Util.h"
#include "WiFiUtil.h"
#include "EEUtil.h"

String serialInput;

//
void printSyntaxHelp()
{
  Serial.print((char)27);
  Serial.print("[2J");
  Serial.println("COMMANDS TABLE");
  for (int i = 0; i < 72; ++i)
    Serial.print('-');
  Serial.println();
  Serial.printf("%-30s | Set WiFi access point id\n", "set wifi ssid <ssid>");
  Serial.printf("%-30s | Set WiFi access point pwd\n", "set wifi pwd <pwd>");
  Serial.printf("%-30s | Reconnect WiFi\n", "reconnect");
  Serial.printf("%-30s | Reset to factory defaults\n", "factoryDefaults");
  Serial.println();
}

//
void processSerialCmd()
{
  if (serialInput.indexOf("set wifi ssid ") == 0)
  {
    strncpy(config.wifiSSID, serialInput.substring(14).c_str(), CONFIG_WIFI_SSID_STR_SIZE);
    saveConfig();    
  }
  else if (serialInput.indexOf("set wifi pwd ") == 0)
  {
    strncpy(config.wifiPwd, serialInput.substring(13).c_str(), CONFIG_WIFI_SSID_PWD_STR_SIZE);
    saveConfig();
  }
  else if (serialInput == "reconnect")
  {
    reconnectWifi();
  }
  else if (serialInput == "factoryDefaults")
  {
    EEResetToFactoryDefault(&config);
    saveConfig();
  }
  else
  {
    Serial.printf("unrecognized cmd [%s]\n", serialInput.c_str());
  }

  serialInput = "";
}