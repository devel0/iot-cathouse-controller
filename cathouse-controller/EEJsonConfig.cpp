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
        .property("standbyDurationMs", standbyDurationMs)
        .property("standbyPort", standbyPort)
        .property("fullpowerDurationMs", fullpowerDurationMs)
        .property("texternGTESysOff", texternGTESysOff)
        .property("adcWeightDeltaCat", adcWeightDeltaCat)
        .property("manualMode", manualMode)
        .property("fanlessMode", fanlessMode)
        .property("portDurationMs", portDurationMs)
        .property("portOverlapDurationMs", portOverlapDurationMs)
        .property("tbottomGTEFanOn", tbottomGTEFanOn);

    if (forWebapi) // saveconfig webapi includes view of these variabiles coming from eeStaticConfig
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

    Serial.printf("printing eeJsonConfig [%s]\n", output.c_str());

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
    tbottomLimit = FACTORY_TBOTTOM_LIMIT;
    twoodLimit = FACTORY_TWOOD_LIMIT;
    tambientLimit = FACTORY_TAMBIENT_LIMIT;
    cooldownTimeMs = FACTORY_COOLDOWN_TIME_MS;
    standbyDurationMs = FACTORY_STANDBY_DURATION_MS;
    standbyPort = FACTORY_STANDBY_PORT;
    fullpowerDurationMs = FACTORY_FULLPOWER_DURATION_MS;
    texternGTESysOff = FACTORY_TEXTERN_GTE_SYS_OFF;
    adcWeightDeltaCat = FACTORY_ADC_WEIGHT_DELTA_CAT;
    tbottomGTEFanOn = FACTORY_TBOTTOM_GTE_FAN_ON;
    manualMode = FACTORY_MANULA_MODE;
    fanlessMode = FACTORY_FANLESS_MODE;
    portDurationMs = FACTORY_PORT_DURATION_MS;
    portOverlapDurationMs = FACTORY_PORT_OVERLAP_DURATION_MS;
}

//-----------------------------------------
// EEPrinter
//-----------------------------------------

EEPrinter::EEPrinter(int _offset)
{
    offset = _offset;
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
            if (strncmp(eeStaticConfig.tbottomId, "\"", DS18B20_ID_STRLENMAX) == 0)
            {
                Serial.printf("skip invalid tbottomId [%s] string contains doublequotes\n", value.c_str());
            }
            else
            {
                strncpy(eeStaticConfig.tbottomId, value.c_str(), DS18B20_ID_STRLENMAX);
                eeStaticConfig.tbottomId[DS18B20_ID_STRLENMAX] = 0;
                eeStaticConfigDirty = true;
            }
        }
    }
    else if (lastKey == "twoodId")
    {
        if (strncmp(eeStaticConfig.twoodId, value.c_str(), DS18B20_ID_STRLENMAX) != 0)
        {
            if (strncmp(eeStaticConfig.twoodId, "\"", DS18B20_ID_STRLENMAX) == 0)
            {
                Serial.printf("skip invalid twoodId [%s] string contains doublequotes\n", value.c_str());
            }
            else
            {
                strncpy(eeStaticConfig.twoodId, value.c_str(), DS18B20_ID_STRLENMAX);
                eeStaticConfig.twoodId[DS18B20_ID_STRLENMAX] = 0;
                eeStaticConfigDirty = true;
            }
        }
    }
    else if (lastKey == "tambientId")
    {
        if (strncmp(eeStaticConfig.tambientId, value.c_str(), DS18B20_ID_STRLENMAX) != 0)
        {
            if (strncmp(eeStaticConfig.tambientId, "\"", DS18B20_ID_STRLENMAX) == 0)
            {
                Serial.printf("skip invalid tambientId [%s] string contains doublequotes\n", value.c_str());
            }
            else
            {
                strncpy(eeStaticConfig.tambientId, value.c_str(), DS18B20_ID_STRLENMAX);
                eeStaticConfig.tambientId[DS18B20_ID_STRLENMAX] = 0;
                eeStaticConfigDirty = true;
            }
        }
    }
    else if (lastKey == "texternId")
    {
        if (strncmp(eeStaticConfig.texternId, value.c_str(), DS18B20_ID_STRLENMAX) != 0)
        {
            if (strncmp(eeStaticConfig.texternId, "\"", DS18B20_ID_STRLENMAX) == 0)
            {
                Serial.printf("skip invalid texternId [%s] string contains doublequotes\n", value.c_str());
            }
            else
            {
                strncpy(eeStaticConfig.texternId, value.c_str(), DS18B20_ID_STRLENMAX);
                eeStaticConfig.texternId[DS18B20_ID_STRLENMAX] = 0;
                eeStaticConfigDirty = true;
            }
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
    else if (lastKey == "standbyDurationMs")
        eeJsonConfig.standbyDurationMs = atol(value.c_str());
    else if (lastKey == "standbyPort")
        eeJsonConfig.standbyPort = atoi(value.c_str());
    else if (lastKey == "fullpowerDurationMs")
        eeJsonConfig.fullpowerDurationMs = atol(value.c_str());
    else if (lastKey == "texternGTESysOff")
        eeJsonConfig.texternGTESysOff = atof(value.c_str());
    else if (lastKey == "adcWeightDeltaCat")
        eeJsonConfig.adcWeightDeltaCat = atoi(value.c_str());
    else if (lastKey == "tbottomGTEFanOn")
        eeJsonConfig.tbottomGTEFanOn = atof(value.c_str());
    else if (lastKey == "manualMode")
        eeJsonConfig.manualMode = (value == "true") ? true : false;
    else if (lastKey == "fanlessMode")
        eeJsonConfig.fanlessMode = (value == "true") ? true : false;
    else if (lastKey == "portDurationMs")
        eeJsonConfig.portDurationMs = atol(value.c_str());
    else if (lastKey == "portOverlapDurationMs")
        eeJsonConfig.portOverlapDurationMs = atol(value.c_str());

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