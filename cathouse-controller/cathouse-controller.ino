#define ARDUINO 10807

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Util.h"

//------------------------------------------
//
// CHANGE AS NEEDED
//

#include "/home/devel0/security/wifi.h"

#define DEBUG // comment to disable serial output
#define TEMPERATURE_INTERVAL_MS 1000
#define TEMPERATURE_DEVICE_COUNT 2
#define ONE_WIRE_BUS D3

//
//------------------------------------------

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTHEX(x) Serial.print(x, HEX)
#define DEBUG_PRINTF(x, args...) Serial.printf(x, args)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTF(x, args...)
#define DEBUG_PRINTHEX(x)
#define DEBUG_PRINTLN(x)
#endif

#define BUILTIN_LED D4

char ssid[] = WIFI_SSID;
char pass[] = WIFI_KEY;
int status = WL_IDLE_STATUS;

WiFiServer server(80);

#define RELAY_ON LOW
#define RELAY_OFF HIGH

OneWire tempOneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&tempOneWire);
DeviceAddress tempDevAddress[TEMPERATURE_DEVICE_COUNT]; // DeviceAddress defined as uint8_t[8]
int temperatureDeviceCount = 0;
float *temperatures = NULL;

void setup()
{
  Serial.begin(115200);

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

  SetupTemperatureDevices();

  server.begin();
}

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

String header;
bool foundcmd;

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
            
            if (temperatureDeviceCount > 0)
            {
              for (int i = 0; i < temperatureDeviceCount; ++i)
              {
                String q = String("GET /temp/");
                q.concat(i);
                //DEBUG_PRINTF("Searching for [%s]", q.c_str());
                if (header.indexOf(q) >= 0)
                {
                  //DEBUG_PRINTF("client requesting temperature sensor %d with header [%s]\n", i, header.c_str());
                  client.printf("%f", temperatures[i]);
                  client.stop();
                  break;
                }
              }
            }

            if (header.indexOf("GET /help") >= 0)
            {
              client.println("<html><body>");
              client.println("<h1>Api</h1>");
              if (temperatureDeviceCount > 0)
              {
                client.println("<h3>Temperature sensors</h3>");
                client.printf("/temp/i ( read temperature of sensor i=0..%d )", temperatureDeviceCount - 1);
              }
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
