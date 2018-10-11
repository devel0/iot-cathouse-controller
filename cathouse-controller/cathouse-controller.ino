#define SERIAL_DEBUG // comment to disable serial output
#include <my-wifi-key.h>

#define TEMPERATURE_INTERVAL_MS 1000
#define TEMPERATURE_DEVICE_COUNT 2
#define ONE_WIRE_BUS D3

#define RELAY_DEVICE_COUNT (sizeof(RELAY_PORTS) / sizeof(int))
int RELAY_PORTS[] = {D5};

#define LED_PORT D7

//------------------------------------------

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Util.h"

#define BUILTIN_LED D4

char ssid[] = WIFI_SSID;
char pass[] = WIFI_KEY;
int status = WL_IDLE_STATUS;

WiFiServer server(80);

OneWire tempOneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&tempOneWire);
DeviceAddress tempDevAddress[TEMPERATURE_DEVICE_COUNT]; // DeviceAddress defined as uint8_t[8]
int temperatureDeviceCount = 0;
float *temperatures = NULL;

#define RELAY_ON LOW
#define RELAY_OFF HIGH

//
// MAIN SETUP
//
void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif

  if (WiFi.status() == WL_NO_SHIELD)
  {
    DEBUG_PRINTLN("WiFi shield not present");
    while (true)
      ;
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    DEBUG_PRINT(".");
  }

  printCurrentNet();
  printWifiData();

  DEBUG_PRINTLN("Startup server");
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  server.begin();

  SetupTemperatureDevices();
  SetupRelays();
  SetupLed();
}

//
// TEMPERATURE SETUP
//
void SetupTemperatureDevices()
{
  DS18B20.begin();
  temperatureDeviceCount = DS18B20.getDeviceCount();
  DEBUG_PRINTF("temperature device count = %d\n", temperatureDeviceCount);
  if (temperatureDeviceCount > 0)
    temperatures = new float[temperatureDeviceCount];
  for (int i = 0; i < temperatureDeviceCount; ++i)
  {
    DS18B20.getAddress(tempDevAddress[i], i);
    DS18B20.setResolution(12);
  }
  ReadTemperatures();
}

unsigned long lastTemperatureRead;

//
// TEMPERATURE READ
//
void ReadTemperatures()
{
  //DEBUG_PRINTLN("reading temperatures");
  DS18B20.requestTemperatures();
  for (int i = 0; i < temperatureDeviceCount; ++i)
  {
    auto temp = DS18B20.getTempC(tempDevAddress[i]);
    DEBUG_PRINTF("temperature sensor %d = %f\n", i, temp);
    temperatures[i] = temp;
  }
  lastTemperatureRead = millis();
}

//
// SETUP RELAYS
//
void SetupRelays()
{
  for (int i = 0; i < RELAY_DEVICE_COUNT; ++i)
  {
    digitalWrite(RELAY_PORTS[i], RELAY_OFF);
    pinMode(RELAY_PORTS[i], OUTPUT);
  }
}

//
// SETUP LED PORT
//
void SetupLed()
{
  digitalWrite(LED_PORT, LOW);
  pinMode(LED_PORT, OUTPUT);
}

String header;
bool foundcmd;

//
// MAIN LOOP
//
void loop()
{
  auto client = server.available();

  if (client)
  {
    DEBUG_PRINTLN("new client");
    String currentLine = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        //DEBUG_PRINTF("read char [%c]\n", c);
        header += c;
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            foundcmd = false;

            //--------------------------
            // TEMPERATURE
            //--------------------------
            if (temperatureDeviceCount > 0)
            {
              for (int i = 0; i < temperatureDeviceCount; ++i)
              {
                String q = String("GET /temp/");
                q.concat(i);
                if (header.indexOf(q) >= 0)
                {
                  client.printf("%f", temperatures[i]);
                  client.stop();
                  break;
                }
              }
            }

            //--------------------------
            // RELAYS
            //--------------------------
            if (RELAY_DEVICE_COUNT > 0)
            {
              for (int i = 0; i < RELAY_DEVICE_COUNT; ++i)
              {
                String q = String("GET /relay/");
                q.concat(i);

                String qon = String(q.c_str());
                qon.concat("/on");

                String qoff = String(q.c_str());
                qoff.concat("/off");

                String qquery = String(q.c_str());
                qquery.concat("/query");

                auto ron = header.indexOf(qon) >= 0;
                auto roff = header.indexOf(qoff) >= 0;
                auto rquery = header.indexOf(qquery) >= 0;

                if (rquery)
                {
                  client.printf("%d", digitalRead(RELAY_PORTS[i]));
                  client.stop();
                  break;
                }
                else if (ron || roff)
                {
                  client.printf("triggering relay to [%s]", ron ? "ON" : "OFF");
                  digitalWrite(RELAY_PORTS[i], ron ? RELAY_ON : RELAY_OFF);
                  client.stop();
                  break;
                }
              }
            }

            //--------------------------
            // LED
            //--------------------------
            {
              String q = String("GET /led");

              String qon = String(q.c_str());
              qon.concat("/on");

              String qoff = String(q.c_str());
              qoff.concat("/off");

              String qquery = String(q.c_str());
              qquery.concat("/query");

              auto ron = header.indexOf(qon) >= 0;
              auto roff = header.indexOf(qoff) >= 0;
              auto rquery = header.indexOf(qquery) >= 0;

              if (rquery)
              {
                client.printf("%d", digitalRead(LED_PORT));
                client.stop();
                break;
              }
              else if (ron || roff)
              {
                client.printf("triggering led to [%s]", ron ? "ON" : "OFF");
                digitalWrite(LED_PORT, ron ? HIGH : LOW);
                client.stop();
                break;
              }
            }

            //--------------------------
            // HELP
            //--------------------------
            if (header.indexOf("GET /") >= 0)
            {
              client.println("<script>function httpGet(theUrl)");
              client.println("{");
              client.println("    var xmlHttp = new XMLHttpRequest();");
              client.println("    xmlHttp.open( \"GET\", theUrl, false );");
              client.println("    xmlHttp.send( null );");
              client.println("    return xmlHttp.responseText;");
              client.println("}</script>");
              client.println("<html><body>");

              // interactive
              client.println("<h1>Cathouse</h1>");
              if (temperatureDeviceCount > 0)
              {
                client.println("<table><thead><tr><td><b>Temp Sensor</b></td><td><b>Value (C)</b></td><td><b>Action</b></td></tr></thead>");
                client.println("<tbody>");
                for (int i = 0; i < temperatureDeviceCount; ++i)
                  client.printf("<tr><td>%d</td><td>%f</td><td><button onclick='location.reload();'>reload</button></td></tr>", i, temperatures[i]);
                client.println("</tbody></table>");
              }

              client.println("<table><thead><tr><td><b>Device</b></td><td><b>Status</b></td><td><b>Action</b></td></tr></thead>");
              client.println("<tbody>");
              for (int i = 0; i < RELAY_DEVICE_COUNT; ++i)
                client.printf("<tr><td>relay %d</td><td>%s</td><td><button onclick='httpGet(\"/relay/0/%s\");location.reload();'>trigger</button></td></tr>",
                              i,
                              digitalRead(RELAY_PORTS[i]) == LOW ? "ON" : "OFF",
                              digitalRead(RELAY_PORTS[i]) == LOW ? "off" : "on");
              client.printf("<tr><td>led</td><td>%s</td><td><button onclick='httpGet(\"/led/%s\");location.reload();'>trigger</button></td></tr>",
                            digitalRead(LED_PORT) == LOW ? "OFF" : "ON",
                            digitalRead(LED_PORT) == LOW ? "on" : "off");
              client.println("</tbody></table>");

              // api
              client.println("<h3>Api</h3>");
              if (temperatureDeviceCount > 0)
              {
                client.println("<h3>Temperature sensors</h3>");
                client.printf("<code>/temp/i</code> ( read temperature of sensor i=0..%d )<br/>", temperatureDeviceCount - 1);
              }
              client.printf("<code>/relay/i/[on|off|query]</code> ( activate/disactivate/query relay i=0..%d )<br/>", RELAY_DEVICE_COUNT - 1);
              client.printf("<code>/led/[on|off|query]</code><br/>");

              client.println("</body></html>");
            }

            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }
      }
    }

    client.stop();
    DEBUG_PRINTLN("Client disconnected");
    header = "";
  }
  else
  {
    if (TimeDiff(lastTemperatureRead, millis()) > TEMPERATURE_INTERVAL_MS)
    {
      ReadTemperatures();
    }
  }
}

//
void printWifiData()
{
  auto ip = WiFi.localIP();
  DEBUG_PRINT("IP Address: ");
  DEBUG_PRINTLN(ip);

  byte mac[6];
  WiFi.macAddress(mac);
  DEBUG_PRINT("MAC address: ");
  for (int i = 0; i < 6; ++i)
  {
    DEBUG_PRINTHEX(mac[i]);
    if (i != 5)
      DEBUG_PRINT(":");
    else
      DEBUG_PRINTLN();
  }
}

//
void printCurrentNet()
{
  DEBUG_PRINTF("SSID: %s\n", WiFi.SSID().c_str());

  auto bssid = WiFi.BSSID();
  DEBUG_PRINT("BSSID: ");
  for (int i = 0; i < 6; ++i)
  {
    DEBUG_PRINTHEX(bssid[i]);
    if (i != 5)
      DEBUG_PRINT(":");
    else
      DEBUG_PRINTLN();
  }

  auto rssi = WiFi.RSSI();
  DEBUG_PRINTF("signal strength (RSSI): %ld\n", rssi);
}
