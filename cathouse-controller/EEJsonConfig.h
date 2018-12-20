#ifndef _EE_JSON_CONFIG_H_
#define _EE_JSON_CONFIG_H_

#include <Arduino.h>
#include "SList.h"
#include "EEStaticConfig.h"
#include "Util.h"

// https://github.com/squix78/json-streaming-parser
#include "JsonListener.h"

#define EE_JSON_CONFIG_OFFSET (FIRMWARE_VERSION_STRLENMAX + WIFI_SSID_STRLENMAX + WIFI_PWD_STRLENMAX)

//
// Follow config can be written and readed from eeprom directly in json format through serialize/deserialize methods
//
class EEJsonConfig
{
public:
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

  // if textern >= texternGTESysOff system enter disable state
  double texternGTESysOff;

  // if adc weight (mean) >= adcWeightGTECatInThere then `cat is in there`
  int adcWeightGTECatInThere;

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