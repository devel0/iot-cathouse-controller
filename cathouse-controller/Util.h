#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef SERIAL_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTHEX(x) Serial.print(x, HEX)
#define DEBUG_PRINTF(x, args...) Serial.printf(x, args)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTF(x, args...)
#define DEBUG_PRINTHEX(x)
#define DEBUG_PRINTLN(x)
#endif

// Compute time delta (ms) between given `now' and reference `start'.
// Pre: `start' must be a value of time taken from millis()
// effectively before the `now.
unsigned long TimeDiff(unsigned long start, unsigned long now);

#endif