#ifndef _STATS_H_
#define _STATS_H_

#include <Arduino.h>

extern uint32_t freeram_min, freeram;
// used for consumption stat
extern double runtime_hr;
extern double Wh;
extern bool p_was_active[4];

void statsInit();
void statsUpdate();

#endif