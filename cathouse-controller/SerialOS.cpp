#include "SerialOS.h"

#include <Arduino.h>

#include "Util.h"
#include "WiFiUtil.h"
#include "EEStaticConfig.h"
#include "EEJsonConfig.h"

String serialInput;

bool serialOsActivated = false;

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
  Serial.printf("%-30s | Reset eeprom to defaults\n", "factoryreset");
  Serial.printf("%-30s | Save current json config to eeprom\n", "savejsoncfg");
  Serial.printf("%-30s | Load json config from eeprom\n", "loadjsoncfg");
  Serial.printf("%-30s | Exit from SerialOs\n", "exit");
  Serial.println();
}

//
void processSerialCmd()
{
  if (serialInput.indexOf("set wifi ssid ") == 0)
  {
    memset(eeStaticConfig.wifiSSID, 0, WIDI_SSID_STRLENMAX + 1);
    strncpy(eeStaticConfig.wifiSSID, serialInput.substring(14).c_str(), WIDI_SSID_STRLENMAX);
    saveEEStaticConfig();
  }
  else if (serialInput.indexOf("set wifi pwd ") == 0)
  {
    memset(eeStaticConfig.wifiPwd, 0, WIDI_PWD_STRLENMAX + 1);
    strncpy(eeStaticConfig.wifiPwd, serialInput.substring(13).c_str(), WIDI_PWD_STRLENMAX);
    saveEEStaticConfig();
  }
  else if (serialInput == "factoryreset")
  {
    Serial.println("clearing eeJsonConfig");
    eeJsonConfig.Clear();
    eeJsonConfig.SaveToEEProm();
  }
  else if (serialInput == "savejsoncfg")
  {
    eeJsonConfig.SaveToEEProm();
  }
  else if (serialInput == "loadjsoncfg")
  {
    eeJsonConfig.LoadFromEEProm();
  }
  else if (serialInput == "reconnect")
  {
    reconnectWifi();
    serialOsActivated = false;
  }
  else if (serialInput == "exit")
  {
    serialOsActivated = false;
  }
  else
  {
    Serial.printf("unrecognized cmd [%s]\n", serialInput.c_str());
  }

  serialInput = "";
}