#include "WiFiMain.h"
#include "WiFiUtil.h"
#include "Config.h"
#include "Util.h"
#include "EEJsonConfig.h"
#include "SList.h"
#include "TempDev.h"
#include "Stats.h"
#include "WeightDev.h"
#include "Engine.h"

#include <ArduinoJson.h>

WiFiServer server(WEB_PORT);

String header;

//
// WiFi main cycle
//
bool manageWifi()
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

          //-----------------------------------------------
          // /
          //-----------------------------------------------
          if (header.indexOf("GET / ") >= 0 || header.indexOf("GET /index.htm") >= 0)
          {
            clientOk(client, HTML);

            clientWriteBigString(client,
#include "index.htm.h"
            );
          }
          //-----------------------------------------------
          // /app.js
          //-----------------------------------------------
          else if (header.indexOf("GET /app.js ") >= 0)
          {
            clientOk(client, JAVASCRIPT);

            clientWriteBigString(client,
#include "app.js.h"
            );
          }
          //--------------------------
          // /tempdevices
          //--------------------------
          else if (header.indexOf("GET /tempdevices ") >= 0)
          {
            clientOk(client, JSON);

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
            //Serial.printf("query for device id = %s]\n", tid.c_str());
            bool found = false;

            clientOk(client, TEXT);

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
            clientOk(client, JSON);

            //Serial.printf("temperatureHistoryFillCnt: %d Off:%d\n", temperatureHistoryFillCnt, temperatureHistoryOff);

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
          // /bithistories
          //--------------------------
          else if (header.indexOf("GET /bithistories ") >= 0)
          {
            clientOk(client, JSON);

            client.print('{');
            auto startoff = (temperatureHistoryFillCnt == temperatureHistorySize) ? temperatureHistoryOff : 0;
            auto size = min(temperatureHistoryFillCnt, temperatureHistorySize);

            {
              client.print("\"catInThereHistory\":[");
              auto j = startoff;
              for (int k = 0; k < size; k++)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(catInThereHistory->Get(j++) ? "1" : "0");

                if (k < size - 1)
                  client.print(',');
              }
              client.print("]");
            }

            {
              client.print(",\"p1History\":[");
              auto j = startoff;
              for (int k = 0; k < size; k++)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(p1History->Get(j++) ? "1" : "0");

                if (k < size - 1)
                  client.print(',');
              }
              client.print("]");
            }

            {
              client.print(",\"p2History\":[");
              auto j = startoff;
              for (int k = 0; k < size; k++)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(p2History->Get(j++) ? "1" : "0");

                if (k < size - 1)
                  client.print(',');
              }
              client.print("]");
            }

            {
              client.print(",\"p3History\":[");
              auto j = startoff;
              for (int k = 0; k < size; k++)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(p3History->Get(j++) ? "1" : "0");

                if (k < size - 1)
                  client.print(',');
              }
              client.print("]");
            }

            {
              client.print(",\"p4History\":[");
              auto j = startoff;
              for (int k = 0; k < size; k++)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(p4History->Get(j++) ? "1" : "0");

                if (k < size - 1)
                  client.print(',');
              }
              client.print("]");
            }

            {
              client.print(",\"fanHistory\":[");
              auto j = startoff;
              for (int k = 0; k < size; k++)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(fanHistory->Get(j++) ? "1" : "0");

                if (k < size - 1)
                  client.print(',');
              }
              client.print("]");
            }

            {
              client.print(",\"disabledHistory\":[");
              auto j = startoff;
              for (int k = 0; k < size; k++)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(disabledHistory->Get(j++) ? "1" : "0");

                if (k < size - 1)
                  client.print(',');
              }
              client.print("]");
            }

            {
              client.print(",\"cooldownHistory\":[");
              auto j = startoff;
              for (int k = 0; k < size; k++)
              {
                if (j == temperatureHistorySize)
                  j = 0;
                client.print(cooldownHistory->Get(j++) ? "1" : "0");

                if (k < size - 1)
                  client.print(',');
              }
              client.print("]");
            }

            client.print('}');
          }
          //--------------------------
          // /info
          //--------------------------
          else if (header.indexOf("GET /info ") >= 0)
          {
            clientOk(client, JSON);

            client.print('{');

            client.print(F("\"statIntervalSec\":"));
            client.print(UPDATE_STATS_INTERVAL_MS / 1000);

            client.print(F(", \"freeram\":"));
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

            client.print(F(", \"manualMode\":"));
            client.print(eeJsonConfig.manualMode ? "true" : "false");

            client.print(F(", \"adcWeightArraySize\":"));
            client.print(adcWeightArraySize);

            client.print(F(", \"adcWeightArrayOff\":"));
            client.print(adcWeightArrayOff);

            client.print(F(", \"adcWeightArrayFillCnt\":"));
            client.print(adcWeightArrayFillCnt);

            client.print(F(", \"adcWeightArray\":["));
            {
              auto wstart = adcWeightArrayFillCnt == adcWeightArraySize ? adcWeightArrayOff : 0;
              for (int i = wstart; i < wstart + adcWeightArrayFillCnt; ++i)
              {
                client.printf("%d", adcWeightArray[i % adcWeightArraySize]);
                if (i < wstart + adcWeightArrayFillCnt - 1)
                  client.print(",");
              }
            }
            client.print(F("]"));

            client.print(F(", \"catIsInThere\":"));
            client.print(catInThere ? "true" : "false");

            for (int i = 0; i < 4; ++i)
            {
              client.printf(", \"p%d\": ", i + 1);
              auto port = portToPin(i + 1);
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

            client.printf(", \"temp_history_interval_min\": %f", temperatureHistoryIntervalSec / 60.0);
            client.printf(", \"prev_cycle\": \"%s\"", getCycleStr(prevCycle).c_str());
            client.printf(", \"current_cycle\": \"%s\"", getCycleStr(currentCycle).c_str());

            client.printf(", \"runtime_hr\": %f", runtime_hr);
            client.printf(", \"Wh\": %f", Wh);

            client.print('}');
          }
          //--------------------------
          // /port/get/{1,2,3,4,5,6,7}
          // note: 5 is led, 6 is fan, 7 is weight (0-1023 adc)
          //--------------------------
          else if (header.indexOf("GET /port/get/") >= 0)
          {
            clientOk(client, TEXT);

            auto port = header.substring(14);
            port = port.substring(0, port.indexOf(" "));

            //Serial.printf("port = [%s]\n", port.c_str());

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

            else if (port == "7")
              client.print(analogRead(A0));
          }
          //--------------------------
          // /port/set/{1,2,3,4,5,6}/{0,1}
          // note: 5 is led, 6 is fan
          //--------------------------
          else if (header.indexOf("GET /port/set/") >= 0)
          {
            clientOk(client, TEXT);

            auto str = header.substring(14);
            auto port = str.substring(0, str.indexOf("/"));
            auto mode = str.substring(str.indexOf("/") + 1, str.indexOf(" "));

            //Serial.printf("setting port [%s] to [%s]\n", port.c_str(), mode.c_str());

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

            if (!eeJsonConfig.manualMode && portpin != LED_PIN)
            {
              client.print("DENIED");
            }
            else
            {
              auto v = digitalRead(portpin);
              if (modenr)
                digitalWrite(portpin, HIGH);
              else
                digitalWrite(portpin, LOW);

              Serial.printf("manual> set port %d from %d\n", portnr, v);
              setManual();

              client.print("OK");
            }
          }
          //--------------------------
          // /port/toggle/{1,2,3,4,5,6}
          // note: 5 is led, 6 is fan
          //--------------------------
          else if (header.indexOf("GET /port/toggle/") >= 0)
          {
            clientOk(client, TEXT);

            auto str = header.substring(17);
            auto port = str.substring(0, str.indexOf(" "));

            auto portnr = atoi(port.c_str());

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

            if (!eeJsonConfig.manualMode && portpin != LED_PIN)
            {
              client.print("DENIED");
            }
            else
            {
              auto v = digitalRead(portpin);
              Serial.printf("manual> toggling port %d from %d\n", portnr, v);
              digitalWrite(portpin, (v == LOW) ? HIGH : LOW);
              setManual();

              client.print("OK");
            }
          }
          //--------------------------
          // /setcatinthere/{0,1}
          //--------------------------
          else if (header.indexOf("GET /setcatinthere/") >= 0)
          {
            clientOk(client, TEXT);

            auto arg = header.substring(19);
            auto str = arg.substring(0, arg.indexOf(" "));

            if (str == "0")
              catInThere = false;
            else
              catInThere = true;

            Serial.printf("engine> override catInThere = %d\n", catInThere ? 1 : 0);

            client.print("OK");
          }
          //-----------------------------------------------
          // /getconfig
          //-----------------------------------------------
          else if (header.indexOf("GET /getconfig ") >= 0)
          {
            //WiFiPrinter prn(client);

            clientOk(client, JSON);

            eeJsonConfig.Save(client, true);
          }
          //-----------------------------------------------
          // /saveconfig
          //-----------------------------------------------
          else if (header.indexOf("POST /saveconfig ") >= 0)
          {
            clientOk(client, TEXT);

            String s;
            while (client.available() || (client.peek() != -1)) // read all frames
            {
              while (client.available())
              {
                auto c = (char)client.read();
                s.concat(c);
              }
              client.flush();
            }
            Serial.printf("saving config [%s]\n", s.c_str());
            
            eeJsonConfig.Load(s.c_str());
            eeJsonConfig.SaveToEEProm();
            if (eeStaticConfigDirty)
            {
              saveEEStaticConfig();
              eeStaticConfigDirty = false;
            }
          }

          header = "";
          break;
        }
      }
    }

    client.stop();

    return true;
  }

  client.stop();

  return false;
}
