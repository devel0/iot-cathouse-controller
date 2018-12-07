#include "WiFiUtil.h"
#include "EEUtil.h"
#include "SerialOS.h"
#include "TempDev.h"
#include "Util.h"
#include "Config.h"
#include "Stats.h"
//#include <ArduinoJson.h>

WiFiServer server(WEB_PORT);

//
void reconnectWifi()
{
  if (server.status() != CLOSED)
    server.stop();

  Serial.printf("Trying connecting SSID:[%s]\n", config.wifiSSID);

  WiFi.begin(config.wifiSSID, config.wifiPwd);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true)
      ;
  }

  Serial.println("Connecting WiFi ( press CTRL+C to stop )");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
    if (Serial.available() && Serial.read() == 3)
    {
      printSyntaxHelp();
      return;
    }
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  printCurrentNet();
  printWifiData();

  Serial.println("Type ? for commands");
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  server.begin();
}

#define CCTYPE_HTML 0
#define CCTYPE_JSON 1
#define CCTYPE_TEXT 2
#define CCTYPE_JS 3
#define CCTYPE_PNG 4

void clientEnd(WiFiClient &client)
{
#if ENABLE_CORS == 1
  client.println("Access-Control-Allow-Origin: *");
#endif

  client.println("Connection: close");
  client.println();
}

void clientError(WiFiClient &client)
{
  client.println("HTTP/1.1 400 Bad Request");

  clientEnd(client);
}

void clientOk(WiFiClient &client, int type)
{
  client.println("HTTP/1.1 200 OK");
  switch (type)
  {
  case CCTYPE_HTML:
    Serial.println("[HTML]");
    client.println("Content-Type: text/html");
    break;

  case CCTYPE_JSON:
    Serial.println("[JSON]");
    client.println("Content-Type: application/json");
    break;

  case CCTYPE_TEXT:
    Serial.println("[TEXT]");
    client.println("Content-Type: text/plain");
    break;

  case CCTYPE_JS:
    Serial.println("[JS]");
    client.println("Content-Type: text/javascript");
    break;

  case CCTYPE_PNG:
    Serial.println("[PNG]");
    client.println("Content-Type: image/png");
    break;
  }

  clientEnd(client);
}

#define FSTRBUFSZ 80

// buffered writing of string
void clientWriteBigString(WiFiClient &client, const __FlashStringHelper *str)
{
  auto p = (const char PROGMEM *)str;
  char buf[FSTRBUFSZ + 1];

  auto l = 0;
  while (pgm_read_byte(p + (l++)))
    ;

  for (int i = 0; i < l; i += FSTRBUFSZ)
  {
    auto s = FSTRBUFSZ;
    if (i + FSTRBUFSZ > l)
      s = l - i;
    auto k = 0;
    while (s > 0)
    {
      buf[k++] = pgm_read_byte(p++);
      --s;
    }

    buf[k] = 0;
    client.print(buf);
  }
}

// buffered writing of binary
void clientWriteBinary(WiFiClient &client, unsigned char arr[], unsigned int l)
{
  auto j = 0;
  for (int i = 0; i < l; i += FSTRBUFSZ)
  {
    auto s = FSTRBUFSZ;
    if (i + FSTRBUFSZ > l)
      s = l - i;

    client.write((const uint8_t *)(arr + i), s);
  }
}

void clientWriteBinaryF(WiFiClient &client, const unsigned char arr[], unsigned int l)
{
  const unsigned char *p = arr;
  uint8_t buf[FSTRBUFSZ];

  for (int i = 0; i < l; i += FSTRBUFSZ)
  {
    auto s = FSTRBUFSZ;
    if (i + FSTRBUFSZ > l)
      s = l - i;
    auto k = 0;
    for (int j = 0; j < s; ++j)
    {
      buf[k++] = pgm_read_byte(p++);
    }

    client.write((const uint8_t *)buf, s);
  }
}

String header;

//
void manageWifi()
{
  WiFiClient client = server.available();

  if (client)
  {
    String currentLine = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if (c != '\n')
          header += c;
        else
        {
          Serial.printf("header [%s]\n", header.c_str());
          int newlinecnt = 0;
          while (client.available() || newlinecnt != 2) // discard rest of header
          {
            c = client.read();

            if (c == '\n')
            {
              if (newlinecnt == 1)
                break;
              else
                newlinecnt = 1;
            }
            else if (c != '\r')
              newlinecnt = 0;
          }

          //--------------------------------
          // /
          //--------------------------------
          if (header.indexOf("GET / ") >= 0 || header.indexOf("GET /index.htm") >= 0)
          {
            clientOk(client, CCTYPE_HTML);

            clientWriteBigString(client,
#include "index.htm.h"
            );
          }
          //--------------------------------
          // /app.js
          //--------------------------------
          else if (header.indexOf("GET /app.js ") >= 0)
          {
            clientOk(client, CCTYPE_JS);

            clientWriteBigString(client,
#include "app.js.h"
            );
          }
          //--------------------------
          // /tempdevices
          //--------------------------
          else if (header.indexOf("GET /tempdevices ") >= 0)
          {
            clientOk(client, CCTYPE_JSON);

            client.print(F("{\"tempdevices\":["));
            for (int i = 0; i < temperatureDeviceCount; ++i)
            {
              client.print('"');
              client.print(tempDevHexAddress[i]);
              client.print('"');

              if (i != temperatureDeviceCount - 1)
                client.print(',');
            }
            client.print("]}");
          }
          //--------------------------
          // /temp/{id}
          //--------------------------
          else if (header.indexOf("GET /temp/") >= 0)
          {
            auto tid = header.substring(10);
            tid = tid.substring(0, tid.indexOf(" "));
            Serial.printf("query for device id = %s]\n", tid.c_str());
            bool found = false;

            clientOk(client, CCTYPE_TEXT);

            for (int i = 0; i < temperatureDeviceCount; ++i)
            {
              if (tid.equals(tempDevHexAddress[i]))
              {
                char tmp[20];
                snprintf(tmp, 19, "%f", temperatures[i]);

                client.print(tmp);

                found = true;
                break;
              }
            }

            if (!found)
              client.print(F("not found"));
          }
          //--------------------------
          // /temphistory
          //--------------------------
          else if (header.indexOf("GET /temphistory ") >= 0)
          {
            clientOk(client, CCTYPE_JSON);

            Serial.printf("temperatureHistoryFillCnt: %d Off:%d\n", temperatureHistoryFillCnt, temperatureHistoryOff);

            client.print('[');
            for (int i = 0; i < temperatureDeviceCount; ++i)
            {
              client.print(F("{\""));
              client.print(tempDevHexAddress[i]);
              client.print(F("\":["));
              auto j = (temperatureHistoryFillCnt == temperatureHistorySize) ? temperatureHistoryOff : 0;
              auto size = min(temperatureHistoryFillCnt, temperatureHistorySize);
              for (int k = 0; k < size; ++k)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(temperatureHistory[i][j++]);
                if (k < size - 1)
                  client.print(',');
              }
              client.print(F("]}"));
              if (i != temperatureDeviceCount - 1)
                client.print(',');
            }
            client.print(']');
          }
          //--------------------------
          // /info
          //--------------------------
          else if (header.indexOf("GET /info ") >= 0)
          {
            clientOk(client, CCTYPE_JSON);

            client.print('{');

            client.print(F("\"freeram\":"));
            client.print(freeram);

            client.print(F(", \"freeram_min\":"));
            client.print(freeram_min);

            client.print(F(", \"history_size\":"));
            client.print(temperatureHistorySize);

            client.print(F(", \"history_interval_sec\":"));
            client.print(temperatureHistoryIntervalSec);

            client.print(F(", \"temperatureHistoryFillCnt\":"));
            client.print(temperatureHistoryFillCnt);

            client.print(F(", \"temperatureHistoryOff\":"));
            client.print(temperatureHistoryOff);

            for (int i = 0; i < 4; ++i)
            {
              client.printf(", \"p%d\": ", i + 1);
              auto port = heatPortIndexToPin(i);
              if (digitalRead(port) == HIGH)
                client.print("true");
              else
                client.print("false");
            }

            if (digitalRead(LED_PIN) == HIGH)
              client.printf(", \"led\": true");
            else
              client.printf(", \"led\": false");

            if (digitalRead(FAN_PIN) == HIGH)
              client.printf(", \"fan\": true");
            else
              client.printf(", \"fan\": false");

            client.printf(", \"runtime_hr\": %f", runtime_hr);
            client.printf(", \"Wh\": %f", Wh);

            client.print('}');
          }
          //--------------------------
          // /port/get/{1,2,3,4,5,6}
          // note: 5 is led, 6 is fan
          //--------------------------
          else if (header.indexOf("GET /port/get/") >= 0)
          {
            clientOk(client, CCTYPE_TEXT);

            auto port = header.substring(14);
            port = port.substring(0, port.indexOf(" "));

            Serial.printf("port = [%s]\n", port.c_str());

            if (port == "1")
              client.print(digitalRead(MOSFET_P1));

            else if (port == "2")
              client.print(digitalRead(MOSFET_P2));

            else if (port == "3")
              client.print(digitalRead(MOSFET_P3));

            else if (port == "4")
              client.print(digitalRead(MOSFET_P4));

            else if (port == "5")
              client.print(digitalRead(LED_PIN));

            else if (port == "6")
              client.print(digitalRead(FAN_PIN));
          }
          //--------------------------
          // /port/set/{1,2,3,4,5}/{0,1}
          // note: 5 is led, 6 is fan
          //--------------------------
          else if (header.indexOf("GET /port/set/") >= 0)
          {
            clientOk(client, CCTYPE_TEXT);

            Serial.printf("header = [%s]\n", header.c_str());

            auto str = header.substring(14);
            auto port = str.substring(0, str.indexOf("/"));
            auto mode = str.substring(str.indexOf("/") + 1, str.indexOf(" "));

            Serial.printf("setting port [%s] to [%s]\n", port.c_str(), mode.c_str());

            auto portnr = atoi(port.c_str());
            auto modenr = atoi(mode.c_str());

            int portpin = MOSFET_P1;

            switch (portnr)
            {
            case 1:
              portpin = MOSFET_P1;
              break;
            case 2:
              portpin = MOSFET_P2;
              break;
            case 3:
              portpin = MOSFET_P3;
              break;
            case 4:
              portpin = MOSFET_P4;
              break;
            case 5:
              portpin = LED_PIN;
              break;
            case 6:
              portpin = FAN_PIN;
              break;
            }

            if (modenr)
              digitalWrite(portpin, HIGH);
            else
              digitalWrite(portpin, LOW);

            client.print("OK");
          }
          //
          // /getconfig
          else if (header.indexOf("GET /getconfig ") >= 0)
          {
            clientOk(client, CCTYPE_JSON);

            client.print("{");
            client.printf("\"%s\": \"%s\",", CSTR_firmwareVersion, config.firmwareVersion);
            client.printf("\"%s\": \"%s\",", CSTR_wifiSSID, config.wifiSSID);
            client.printf("\"%s\": %d,", CSTR_temperatureHistoryFreeramThreshold, config.temperatureHistoryFreeramThreshold);
            client.printf("\"%s\": %d,", CSTR_temperatureHistoryBacklogHours, config.temperatureHistoryBacklogHours);
            client.printf("\"%s\": %lu,", CSTR_updateConsumptionIntervalMs, config.updateConsumptionIntervalMs);
            client.printf("\"%s\": %lu,", CSTR_updateFreeramIntervalMs, config.updateFreeramIntervalMs);
            client.printf("\"%s\": %lu,", CSTR_updateTemperatureIntervalMs, config.updateTemperatureIntervalMs);
            client.printf("\"%s\": %f,", CSTR_tbottomLimit, config.tbottomLimit);
            client.printf("\"%s\": %f,", CSTR_twoodLimit, config.twoodLimit);
            client.printf("\"%s\": %f,", CSTR_tambientLimit, config.tambientLimit);
            client.printf("\"%s\": %lu,", CSTR_cooldownTimeMs, config.cooldownTimeMs);
            client.printf("\"%s\": %f,", CSTR_tambientVsExternGTESysOff, config.tambientVsExternGTESysOff);
            client.printf("\"%s\": %f,", CSTR_tambientVsExternLTESysOn, config.tambientVsExternLTESysOn);
            client.printf("\"%s\": %f,", CSTR_tbottomGTEFanOn, config.tbottomGTEFanOn);
            client.printf("\"%s\": %f,", CSTR_tbottomLTEFanOff, config.tbottomLTEFanOff);
            client.printf("\"%s\": %f,", CSTR_autoactivateWoodBottomDeltaGTESysOn, config.autoactivateWoodBottomDeltaGTESysOn);
            client.printf("\"%s\": %f,", CSTR_autodeactivateWoodDeltaLT, config.autodeactivateWoodDeltaLT);
            client.printf("\"%s\": %f,", CSTR_autodeactivateInhibitAutoactivateMinMs, config.autodeactivateInhibitAutoactivateMinMs);
            client.printf("\"%s\": %d,", CSTR_autodeactivateExcursionSampleCount, config.autodeactivateExcursionSampleCount);
            client.printf("\"%s\": %lu,", CSTR_autodeactivateExcursionSampleTotalMs, config.autodeactivateExcursionSampleTotalMs);
            client.printf("\"%s\": %f", CSTR_texternGTESysOff, config.texternGTESysOff);

            client.print("}");
          }
          //
          // /saveconfig
          else if (header.indexOf("POST /saveconfig ") >= 0)
          {
            Serial.printf("freeram = %d\n", freeMemorySum());
            String s;

            while (client.peek() != -1)
            {
              while (client.available())
              {
                auto c = (char)client.read();
                s.concat(c);
              }
              client.flush();              
            }

            Serial.printf("saving config [%s]\n", s.c_str());
            int i = 0;
            int len = s.length();
            const char *str = s.c_str();
            while (i < len && str[i++] != '{')
              ;
            while (i < len)
            {
              while (i < len && str[i++] != '"')
                ;
              String name;
              while (i < len && str[i] != '"')
                name.concat(str[i++]);
              String value;
              while (i < len && str[i++] != ':')
                ;
              while (i < len && str[i] != ',' && str[i] != '}')
                value.concat(str[i++]);
              Serial.printf("name = [%s] value = [%s]\n", name.c_str(), value.c_str());

              if (strcmp(name.c_str(), CSTR_temperatureHistoryFreeramThreshold) == 0)
                config.temperatureHistoryFreeramThreshold = atoi(value.c_str());

              else if (strcmp(name.c_str(), CSTR_temperatureHistoryBacklogHours) == 0)
                config.temperatureHistoryBacklogHours = atoi(value.c_str());

              else if (strcmp(name.c_str(), CSTR_updateConsumptionIntervalMs) == 0)
                config.updateConsumptionIntervalMs = atoi(value.c_str());

              else if (strcmp(name.c_str(), CSTR_updateFreeramIntervalMs) == 0)
                config.updateFreeramIntervalMs = atoi(value.c_str());

              else if (strcmp(name.c_str(), CSTR_updateTemperatureIntervalMs) == 0)
                config.updateTemperatureIntervalMs = atoi(value.c_str());

              else if (strcmp(name.c_str(), CSTR_tbottomLimit) == 0)
                config.tbottomLimit = atof(value.c_str());
            }
            auto err = false;
            if (i == 0 || str[i - 1] != '}')
              err = true;

            if (!err)
            {
              saveConfig();

              clientOk(client, CCTYPE_JSON);
            }
            else
            {
              clientError(client);
            }
          }

          header = "";
          break;
        }
      }
    }

    Serial.println("---> client stop");
    client.stop();
  }
}

//
void printWifiData()
{
  auto ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  for (int i = 0; i < 6; ++i)
  {
    Serial.print(mac[i], HEX);
    if (i != 5)
      Serial.print(":");
    else
      Serial.println();
  }
}

//
void printCurrentNet()
{
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());

  auto bssid = WiFi.BSSID();
  Serial.print("BSSID: ");
  for (int i = 0; i < 6; ++i)
  {
    Serial.print(bssid[i], HEX);
    if (i != 5)
      Serial.print(":");
    else
      Serial.println();
  }

  auto rssi = WiFi.RSSI();
  Serial.printf("signal strength (RSSI): %ld\n", rssi);
}