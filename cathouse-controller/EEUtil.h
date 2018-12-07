#ifndef _EE_UTIL_H_
#define _EE_UTIL_H_

#define EE_SIZE_MAX 4096

#include <Arduino.h>
#include "Config.h"

void EEInit();

void EEResetToFactoryDefault(Config *data);
void EERead(Config *data);
void EEWrite(Config *data);

#endif