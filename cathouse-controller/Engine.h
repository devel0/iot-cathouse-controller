#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <Arduino.h>

enum CycleTypes
{
    none,
    fullpower,
    standby,
    cooldown,
    disabled,
    fanless
};

extern CycleTypes prevCycle;
extern CycleTypes currentCycle;

void engineProcess();
String getCycleStr(CycleTypes cycle);

#endif