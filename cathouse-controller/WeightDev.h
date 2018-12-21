#ifndef _WEIGHT_DEV_H_
#define _WEIGHT_DEV_H_

#include <Arduino.h>

extern bool catInThere;
extern uint16_t *adcWeightArray;
extern int adcWeightArrayOff;
extern int adcWeightArraySize;
extern int adcWeightArrayFillCnt;

void EvalAdcWeight();

#endif
