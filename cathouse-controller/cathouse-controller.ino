#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <ESP8266HTTPClient.h>

//------------------------------------------
//
// CHANGE AS NEEDED
//

#include "/home/devel0/security/wifi.h"

#define DEBUG // comment to disable serial output

//
//------------------------------------------

#ifdef DEBUG
#define DEBUG_PRINT(x)     Serial.print (x)
#define DEBUG_PRINTHEX(x)     Serial.print (x,HEX)
#define DEBUG_PRINTF(x, args...)     Serial.printf (x, args)
#define DEBUG_PRINTLN(x)  Serial.println (x)
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

//int pins[] = { D0, D1, D2, D3, /*D4,*/ D5, D6 };
#define RELAY_ON LOW
#define RELAY_OFF HIGH

void setup() {  
  Serial.begin(115200);  
    
  if (WiFi.status() == WL_NO_SHIELD) {
    DEBUG_PRINTLN("WiFi shield not present");    
    while (true);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }
    
  printCurrentNet();
  printWifiData();

  DEBUG_PRINTLN("Startup server");
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  server.begin();
}

String header;
bool foundcmd;

void loop() { 
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
/*
            for (int i = 0; i < 6; ++i)
            {
              String header_search;
              pintmpstr[0] = '0' + i;
              header_search += "GET /";
              header_search += pintmpstr;

              if (header.indexOf(header_search + "/on") >= 0)
              {
                digitalWrite(pins[i], RELAY_ON); foundcmd = true;
                DEBUG_PRINT("on ");
                DEBUG_PRINTLN(i);
              }
              else if (header.indexOf(header_search + "/off") >= 0)
              {
                digitalWrite(pins[i], RELAY_OFF); foundcmd = true;
                DEBUG_PRINT("off ");
                DEBUG_PRINTLN(i);
              }
            }            
*/
            client.println("<html><body>");
            client.println("<h1>Api</h1>");
            client.println("/<i>port</i>/on ( activate port )<br/>");
            client.println("/<i>port</i>/off ( deactivate port )<br/>");
            client.println("port=0..5</br>");
            client.println("</body></html>");

            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        } else if (c != '\r')
        {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
    DEBUG_PRINTLN("Client disconnected");
  }
}

void printWifiData() {
  auto ip = WiFi.localIP();
  DEBUG_PRINT("IP Address: ");
  DEBUG_PRINTLN(ip);
  
  byte mac[6];
  WiFi.macAddress(mac);
  DEBUG_PRINT("MAC address: ");
  for (int i = 0; i < 6; ++i)
  {
    DEBUG_PRINTHEX(mac[i]);
    if (i != 5) DEBUG_PRINT(":");
    else DEBUG_PRINTLN();
  }
}

void printCurrentNet() {
  DEBUG_PRINTF("SSID: %s\n", WiFi.SSID().c_str());
  
  auto bssid = WiFi.BSSID();
  DEBUG_PRINT("BSSID: ");
  for (int i = 0; i < 6; ++i)
  {
    DEBUG_PRINTHEX(bssid[i]);
    if (i != 5) DEBUG_PRINT(":");
    else DEBUG_PRINTLN();
  }
  
  auto rssi = WiFi.RSSI();
  DEBUG_PRINTF("signal strength (RSSI): %ld\n", rssi);  
}
