#ifndef _UTIL_H_
#define _UTIL_H_

unsigned long TimeDiff(unsigned long start, unsigned long now);

extern "C" {
#include "user_interface.h"
}
uint32_t FreeMemorySum();

#endif