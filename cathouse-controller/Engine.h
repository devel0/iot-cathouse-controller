#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <Arduino.h>

enum CycleTypes
{
    // initial
    none,

    // heat first phase
    fullpower,
    // heat continuous
    standby,

    // tbottom or twood or tambient exceeded
    cooldown,

    // textern exceeded
    disabled,
    
    manual
};

extern CycleTypes prevCycle;
extern CycleTypes currentCycle;

void engineProcess();
String getCycleStr(CycleTypes cycle);
void setManual();

#endif