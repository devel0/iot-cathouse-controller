#include "EEJsonConfig.h"

#include <EEPROM.h>

// https://github.com/squix78/json-streaming-parser
#include "JsonStreamingParser.h"
#include "JsonListener.h"

// https://gist.github.com/arcao/3252bb6e5e52493f03726ec32e61395c
#include "StringStream.h"

// https://github.com/maxpowel/ArduinoJsonWriter
#include "JsonWriter.h"

//-----------------------------------------
// EEJsonConfig
//-----------------------------------------

EEJsonConfig eeJsonConfig;

void EEJsonConfig::Save(Print &prn)
{
    String output;
    StringStream stream(output);
    JsonWriter json(&stream);

    json.beginObject();

    json.beginObject()
        .property("tbottomID", tbottomID.c_str())
        .property("twoodID", tbottomID.c_str())
        .property("tambientID", tbottomID.c_str())
        .property("texternID", tbottomID.c_str())
        .property("temperatureHistoryFreeramThreshold", temperatureHistoryFreeramThreshold)
        .property("temperatureHistoryBacklogHours", temperatureHistoryBacklogHours)
        .property("updateConsumptionIntervalMs", updateConsumptionIntervalMs)
        .property("updateFreeramIntervalMs", updateFreeramIntervalMs)
        .property("updateTemperatureIntervalMs", updateTemperatureIntervalMs)
        .property("tbottomLimit", tbottomLimit)
        .property("twoodLimit", twoodLimit)
        .property("tambientLimit", tambientLimit)
        .property("cooldownTimeMs", cooldownTimeMs)
        .property("tambientVsExternGTESysOff", tambientVsExternGTESysOff)
        .property("tambientVsExternLTESysOn", tambientVsExternLTESysOn)
        .property("tbottomGTEFanOn", tbottomGTEFanOn)
        .property("tbottomLTEFanOff", tbottomLTEFanOff)
        .property("autoactivateWoodBottomDeltaGTESysOn", autoactivateWoodBottomDeltaGTESysOn)
        .property("autodeactivateWoodDeltaLT", autodeactivateWoodDeltaLT)
        .property("autodeactivateInhibitAutoactivateMinMs", autodeactivateInhibitAutoactivateMinMs)
        .property("autodeactivateExcursionSampleCount", autodeactivateExcursionSampleCount)
        .property("autodeactivateExcursionSampleTotalMs", autodeactivateExcursionSampleTotalMs)
        .property("texternGTESysOff", texternGTESysOff)
        .endObject();

    Serial.printf("saving eeJsonConfig [%s]\n", output.c_str());

    prn.print(output.c_str());
}

void EEJsonConfig::SaveToEEProm()
{
    EEPrinter prn(EE_JSON_CONFIG_OFFSET);
    Save(prn);
    prn.commit();
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
    temperatureHistoryFreeramThreshold = 10 * 1024;
    temperatureHistoryBacklogHours = 48;
    updateConsumptionIntervalMs = 2500;
    updateFreeramIntervalMs = 1000;
    updateTemperatureIntervalMs = 5000;
    tbottomLimit = 40;
    twoodLimit = 50;
    tambientLimit = 17;
    cooldownTimeMs = 2 * 60 * 1000;
    tambientVsExternGTESysOff = 11;
    tambientVsExternLTESysOn = 10;
    tbottomGTEFanOn = 30;
    tbottomLTEFanOff = 25;
    autoactivateWoodBottomDeltaGTESysOn = 3;
    autodeactivateWoodDeltaLT = 8.5;
    autodeactivateInhibitAutoactivateMinMs = 30 * 60 * 1000;
    autodeactivateExcursionSampleCount = 5;
    autodeactivateExcursionSampleTotalMs = 30 * 60 * 1000;
    texternGTESysOff = 14;
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
    if (lastKey == "tbottomID")
        eeJsonConfig.tbottomID = value;
    else if (lastKey == "twoodID")
        eeJsonConfig.twoodID = value;
    else if (lastKey == "tambientID")
        eeJsonConfig.tambientID = value;
    else if (lastKey == "texternID")
        eeJsonConfig.texternID = value;
    else if (lastKey == "temperatureHistoryFreeramThreshold")
        eeJsonConfig.temperatureHistoryFreeramThreshold = atoi(value.c_str());
    else if (lastKey == "temperatureHistoryBacklogHours")
        eeJsonConfig.temperatureHistoryBacklogHours = atoi(value.c_str());
    else if (lastKey == "updateConsumptionIntervalMs")
        eeJsonConfig.updateConsumptionIntervalMs = atol(value.c_str());
    else if (lastKey == "updateFreeramIntervalMs")
        eeJsonConfig.updateFreeramIntervalMs = atol(value.c_str());
    else if (lastKey == "updateTemperatureIntervalMs")
        eeJsonConfig.updateTemperatureIntervalMs = atol(value.c_str());
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
    else if (lastKey == "tbottomGTEFanOn")
        eeJsonConfig.tbottomGTEFanOn = atof(value.c_str());
    else if (lastKey == "tbottomLTEFanOff")
        eeJsonConfig.tbottomLTEFanOff = atof(value.c_str());
    else if (lastKey == "autoactivateWoodBottomDeltaGTESysOn")
        eeJsonConfig.autoactivateWoodBottomDeltaGTESysOn = atof(value.c_str());
    else if (lastKey == "autodeactivateWoodDeltaLT")
        eeJsonConfig.autodeactivateWoodDeltaLT = atof(value.c_str());
    else if (lastKey == "autodeactivateInhibitAutoactivateMinMs")
        eeJsonConfig.autodeactivateInhibitAutoactivateMinMs = atol(value.c_str());
    else if (lastKey == "autodeactivateExcursionSampleCount")
        eeJsonConfig.autodeactivateExcursionSampleCount = atoi(value.c_str());

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