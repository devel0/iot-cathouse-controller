#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <Arduino.h>

enum CycleTypes
{
    // initial
    none,

    // active mode
    active,    

    // tbottom or twood or tambient exceeded
    cooldown,

    // textern exceeded
    disabled,
    
    manual
};

extern CycleTypes prevCycle;
extern CycleTypes currentCycle;
extern bool catInThereOverriden;

int getPorts();
void engineProcess();
String getCycleStr(CycleTypes cycle);
void setManual();
void unsetManual();

#endif