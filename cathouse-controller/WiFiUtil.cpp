#include "WiFiUtil.h"

#include "SerialOS.h"
#include "EEStaticConfig.h"
#include "Util.h"

void reconnectWifi()
{
    if (server.status() != CLOSED)
        server.stop();

    Serial.printf("Trying connecting SSID:[%s]\n", eeStaticConfig.wifiSSID);

    WiFi.begin(eeStaticConfig.wifiSSID, eeStaticConfig.wifiPwd);

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
            serialOsActivated = true;
            Serial.println("SerialOs activated");
            return;
        }
    }

    // you're connected now, so print out the data:
    Serial.println("You're connected to the network");
    printCurrentNet();
    printWifiData();

    Serial.println("Type ? for commands");

    server.begin();

    builtinLedOn();
}

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

void clientOk(WiFiClient &client, CCTypes type)
{
    client.println("HTTP/1.1 200 OK");
    switch (type)
    {
    case HTML:
        Serial.println("[HTML]");
        client.println("Content-Type: text/html");
        break;

    case JSON:
        Serial.println("[JSON]");
        client.println("Content-Type: application/json");
        break;

    case TEXT:
        Serial.println("[TEXT]");
        client.println("Content-Type: text/plain");
        break;

    case JAVASCRIPT:
        Serial.println("[JS]");
        client.println("Content-Type: text/javascript");
        break;

    case PNG:
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