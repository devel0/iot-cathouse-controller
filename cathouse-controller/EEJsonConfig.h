#ifndef _EE_JSON_CONFIG_H_
#define _EE_JSON_CONFIG_H_

#include <Arduino.h>
#include "SList.h"
#include "EEStaticConfig.h"
#include "Util.h"

// https://github.com/squix78/json-streaming-parser
#include "JsonListener.h"

#define EE_JSON_CONFIG_OFFSET (sizeof(EEStaticConfig))

//
// Follow config can be written and readed from eeprom directly in json format through serialize/deserialize methods
//
class EEJsonConfig
{
public:
  // if true ports will not changed automatically by the mcu but can managed through webapi
  bool manualMode;

  // if bottom temp >= tbottomLimit heat ports gets disabled for cooldownTimeMs
  double tbottomLimit;

  // if wood temp >= twoodLimit heat ports gets disabled for cooldownTimeMs
  double twoodLimit;

  // if ambient temp >= tambientLimit heat ports gets disabled for cooldownTimeMs
  double tambientLimit;

  // duration of cooldown if cooldown condition occurs (see tbottomLimit, twoodLimit, tambientLimit)
  unsigned long cooldownTimeMs;

  // duration of standby cycle
  unsigned long standbyDurationMs;

  // port to maintain enabled during standby (1-4 or 0 for none)
  int standbyPort;  

  // duration of fullpower cycle
  unsigned long fullpowerDurationMs;

  // if textern >= texternGTESysOff system enter disable state
  double texternGTESysOff;

  // adc weight quantity delta between mean values (last 20sec) to detect ingress/egress of the cat
  int adcWeightDeltaCat;

  // when in fullpower mode fan activate if tbottom >= T
  double tbottomGTEFanOn;

  // serialize this config in json format using overridable Printer write method
  void Save(Print &prn, bool forWebapi = false);
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