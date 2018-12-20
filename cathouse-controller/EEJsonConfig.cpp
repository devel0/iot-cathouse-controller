#include "EEJsonConfig.h"
#include "EEStaticConfig.h"

#include <EEPROM.h>

// https://github.com/squix78/json-streaming-parser
#include "JsonStreamingParser.h"
#include "JsonListener.h"

// https://gist.github.com/arcao/3252bb6e5e52493f03726ec32e61395c
#include "StringStream.h"

// https://github.com/maxpowel/ArduinoJsonWriter
#include "JsonWriter.h"

#include "Config.h"

//-----------------------------------------
// EEJsonConfig
//-----------------------------------------

EEJsonConfig eeJsonConfig;

void EEJsonConfig::Save(Print &prn, bool forWebapi)
{
    String output;
    StringStream stream(output);
    JsonWriter json(&stream);

    json.beginObject()
        .property("tbottomLimit", tbottomLimit)
        .property("twoodLimit", twoodLimit)
        .property("tambientLimit", tambientLimit)
        .property("cooldownTimeMs", cooldownTimeMs)
        .property("tambientVsExternGTESysOff", tambientVsExternGTESysOff)
        .property("tambientVsExternLTESysOn", tambientVsExternLTESysOn)
        .property("texternGTESysOff", texternGTESysOff)
        .property("adcWeightGTECatInThere", adcWeightGTECatInThere);

    if (forWebapi)
    {
        json.property("firmwareVersion", FIRMWARE_VER)
            .property("wifiSSID", eeStaticConfig.wifiSSID)
            .property("tbottomId", String(eeStaticConfig.tbottomId))
            .property("twoodId", String(eeStaticConfig.twoodId))
            .property("tambientId", String(eeStaticConfig.tambientId))
            .property("texternId", String(eeStaticConfig.texternId));
    }

    json
        .endObject();

    Serial.printf("saving eeJsonConfig [%s]\n", output.c_str());

    prn.print(output);
}

void EEJsonConfig::SaveToEEProm()
{
    EEPrinter prn(EE_JSON_CONFIG_OFFSET);
    Save(prn);
    prn.commit();

    Save(Serial);
}

void EEJsonConfig::Load(const char *json)
{
    Serial.printf("parsing eeJsonConfig [%s]\n", json);

    JsonStreamingParser parser;
    EEJsonConfigParseListener listener;
    parser.setListener(&listener);
    while (*json)
    {
        parser.parse(*(json++));
    }
}

void EEJsonConfig::LoadFromEEProm()
{
    String s;
    int i = EE_JSON_CONFIG_OFFSET;
    while (i < EE_SIZE)
    {
        char c = EEPROM.read(i);
        if (c)
        {
            s.concat(c);
            ++i;
        }
        else
            break;
    }
    Load(s.c_str());
}

void EEJsonConfig::Clear()
{
    tbottomLimit = 40;
    twoodLimit = 50;
    tambientLimit = 17;
    cooldownTimeMs = 2 * 60 * 1000;
    tambientVsExternGTESysOff = 11;
    tambientVsExternLTESysOn = 10;
    texternGTESysOff = 14;
    adcWeightGTECatInThere = 260;
}

//-----------------------------------------
// EEPrinter
//-----------------------------------------

EEPrinter::EEPrinter(int offset)
{
    this->offset = offset;
}

size_t EEPrinter::write(uint8_t c)
{
    if (offset < EE_SIZE)
        EEPROM.write(offset, c);

    offset++;

    return 1;
}

void EEPrinter::commit()
{
    if (offset > EE_SIZE)
    {
        Serial.printf("fail to save eeJsonConfig ( too big > %d )\n", EE_SIZE);
    }
    else
    {
        EEPROM.write(offset++, 0); // null terminating
        EEPROM.commit();
    }
}

//-----------------------------------------
// EEJsonConfigParseListener
//-----------------------------------------

enum ParserStatus
{
    none,
    tempSensors,
    relays,
    triggers
};

ParserStatus parserStatus = none;

void EEJsonConfigParseListener::whitespace(char c) {}

void EEJsonConfigParseListener::startDocument()
{
    eeJsonConfig.Clear();
}

void EEJsonConfigParseListener::startArray() {}

void EEJsonConfigParseListener::startObject() {}

String lastKey;
void EEJsonConfigParseListener::key(String key)
{
    /*if (parserStatus == none)
    {
        if (key == "temp_sensors")
            parserStatus = tempSensors;
        else if (key == "relays")
            parserStatus = relays;
        else if (key == "triggers")
            parserStatus = triggers;
    }*/
    lastKey = key;
    //Serial.println("key: " + key);
}

void EEJsonConfigParseListener::value(String value)
{
    if (lastKey == "tbottomId")
    {
        if (strncmp(eeStaticConfig.tbottomId, value.c_str(), DS18B20_ID_STRLENMAX) != 0)
        {
            strncpy(eeStaticConfig.tbottomId, value.c_str(), DS18B20_ID_STRLENMAX);
            eeStaticConfig.tbottomId[DS18B20_ID_STRLENMAX] = 0;
            eeStaticConfigDirty = true;
        }
    }
    else if (lastKey == "twoodId")
    {
        if (strncmp(eeStaticConfig.twoodId, value.c_str(), DS18B20_ID_STRLENMAX) != 0)
        {
            strncpy(eeStaticConfig.twoodId, value.c_str(), DS18B20_ID_STRLENMAX);
            eeStaticConfig.twoodId[DS18B20_ID_STRLENMAX] = 0;
            eeStaticConfigDirty = true;
        }
    }
    else if (lastKey == "tambientId")
    {
        if (strncmp(eeStaticConfig.tambientId, value.c_str(), DS18B20_ID_STRLENMAX) != 0)
        {
            strncpy(eeStaticConfig.tambientId, value.c_str(), DS18B20_ID_STRLENMAX);
            eeStaticConfig.tambientId[DS18B20_ID_STRLENMAX] = 0;
            eeStaticConfigDirty = true;
        }
    }
    else if (lastKey == "texternId")
    {
        if (strncmp(eeStaticConfig.texternId, value.c_str(), DS18B20_ID_STRLENMAX) != 0)
        {
            strncpy(eeStaticConfig.texternId, value.c_str(), DS18B20_ID_STRLENMAX);
            eeStaticConfig.texternId[DS18B20_ID_STRLENMAX] = 0;
            eeStaticConfigDirty = true;
        }
    }
    else if (lastKey == "tbottomLimit")
        eeJsonConfig.tbottomLimit = atof(value.c_str());
    else if (lastKey == "twoodLimit")
        eeJsonConfig.twoodLimit = atof(value.c_str());
    else if (lastKey == "tambientLimit")
        eeJsonConfig.tambientLimit = atof(value.c_str());
    else if (lastKey == "cooldownTimeMs")
        eeJsonConfig.cooldownTimeMs = atol(value.c_str());
    else if (lastKey == "tambientVsExternGTESysOff")
        eeJsonConfig.tambientVsExternGTESysOff = atof(value.c_str());
    else if (lastKey == "tambientVsExternLTESysOn")
        eeJsonConfig.tambientVsExternLTESysOn = atof(value.c_str());
    else if (lastKey == "texternGTESysOff")
        eeJsonConfig.texternGTESysOff = atof(value.c_str());
    else if (lastKey == "adcWeightGTECatInThere")
        eeJsonConfig.adcWeightGTECatInThere = atoi(value.c_str());

    //Serial.println("value: " + value);
}

void EEJsonConfigParseListener::endObject()
{
}

void EEJsonConfigParseListener::endArray()
{
    parserStatus = none;
}

void EEJsonConfigParseListener::endDocument() {}