#ifndef _EE_JSON_CONFIG_H_
#define _EE_JSON_CONFIG_H_

#include <Arduino.h>
#include "SList.h"
#include "EEStaticConfig.h"
#include "Util.h"

// https://github.com/squix78/json-streaming-parser
#include "JsonListener.h"

#define EE_JSON_CONFIG_OFFSET (FIRMWARE_VERSION_STRLENMAX + WIDI_SSID_STRLENMAX + WIDI_PWD_STRLENMAX)

//
// Follow config can be written and readed from eeprom directly in json format through serialize/deserialize methods
//
class EEJsonConfig
{
public:
  String tbottomId;
  String twoodId;
  String tambientId;
  String texternId;  

  // allocate temperature history so that it can hold temperatureHistoryBacklogHours hours
  // leaving temperatureHistoryFreeramThreshold free ram bytes
  unsigned int temperatureHistoryFreeramThreshold;

  // (see temperatureHistoryFreeramThreshold )
  unsigned int temperatureHistoryBacklogHours;

  // ms interval to update Wh consumption stat
  unsigned long updateConsumptionIntervalMs;

  // ms interval to update freeram stat
  unsigned long updateFreeramIntervalMs;

  // ms interval to update current temperature
  unsigned long updateTemperatureIntervalMs;

  // if bottom temp >= tbottomLimit heat ports gets disabled for cooldownTimeMs
  double tbottomLimit;

  // if wood temp >= twoodLimit heat ports gets disabled for cooldownTimeMs
  double twoodLimit;

  // if ambient temp >= tambientLimit heat ports gets disabled for cooldownTimeMs
  double tambientLimit;

  // heat ports disable time when cooldown condition occurs (see tbottomLimit, twoodLimit, tambientLimit)
  unsigned long cooldownTimeMs;

  // if tambient >= tambientVsExternGTESysOff heat ports enter disable state
  double tambientVsExternGTESysOff;

  // if tambient <= tambientVsExternLTESysOn heat ports enter enable state
  double tambientVsExternLTESysOn;

  // if tbottom >= tbottomGTEFanOn fan enter enable state
  double tbottomGTEFanOn;

  // if tbottom <= tbottomLTEFanOff fan enter disable state
  double tbottomLTEFanOff;

  // if (twood - tbottom) >= autoactivateWoodBottomDeltaGTESysOn system enter enable state
  double autoactivateWoodBottomDeltaGTESysOn;

  // if tbottom excursion over last autodeactivateExcursionSampleTotalMs interval <= autodeactivateWoodDeltaLT
  // system enter disable state for atleast autodeactivateInhibitAutoactivateMinMs timespan
  double autodeactivateWoodDeltaLT;

  // (see autodeactivateWoodDeltaLT)
  unsigned long autodeactivateInhibitAutoactivateMinMs;

  // (see also autodeactivateWoodDeltaLT)
  // stat past autodeactivateExcursionSampleTotalMs temperatures time interval using
  // a bunch of autodeactivateExcursionSampleCount (min,max) objects
  // when total stat interval exceeded oldest sample removed and added a new one on top (current time)
  int autodeactivateExcursionSampleCount;

  // (see autodeactivateExcursionSampleCount)
  // suggested value : at least 1.5 x heat cycle time interval
  unsigned long autodeactivateExcursionSampleTotalMs;

  // if textern >= texternGTESysOff system enter disable state
  double texternGTESysOff;

  // serialize this config in json format using overridable Printer write method
  void Save(Print &prn);
  void SaveToEEProm();

  // deserialize given json data into this object
  void Load(const char *json);
  void LoadFromEEProm();
  // clear config ram content
  void Clear();
};

extern EEJsonConfig eeJsonConfig;

class EEPrinter : public Print
{
  int offset;

public:
  EEPrinter(int offset);
  size_t write(uint8_t c);
  void commit();
};

class EEJsonConfigParseListener : public JsonListener
{
  String currentArray;

public:
  virtual void whitespace(char c);
  virtual void startDocument();
  virtual void startArray();
  virtual void startObject();
  virtual void key(String key);
  virtual void value(String value);
  virtual void endObject();
  virtual void endArray();
  virtual void endDocument();
};

#endif