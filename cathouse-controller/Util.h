#ifndef _UTIL_H_
#define _UTIL_H_

#include <Arduino.h>

unsigned long timeDiff(unsigned long start, unsigned long now);

extern "C" {
#include "user_interface.h"
}
uint32_t freeMemorySum();

String trimDoubleQuotes(String str);

void builtinLedOn();
void builtinLedOff();

#endif