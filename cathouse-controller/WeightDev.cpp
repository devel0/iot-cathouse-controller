#include "WeightDev.h"
#include "Util.h"
#include "SList.h"
#include "Config.h"
#include "EEJsonConfig.h"

bool catInThere = false;
uint16_t *adcWeightArray = NULL;
int adcWeightArrayOff = 0;
int adcWeightArraySize = 0;
int adcWeightArrayFillCnt = 0;

double adcWeight = 0.0;

void EvalAdcWeight()
{
    if (adcWeightArray == NULL)
    {
        adcWeightArraySize = ADCWEIGHT_HISTORY_BACKLOG_KB * 1024 / sizeof(uint16_t);
        adcWeightArray = (uint16_t *)malloc(sizeof(uint16_t) * adcWeightArraySize);
    }

    adcWeight = analogRead(ADCWEIGHT_PIN);

    if (adcWeightArrayOff == adcWeightArraySize)
        adcWeightArrayOff = 0;

    adcWeightArray[adcWeightArrayOff++] = adcWeight;

    if (adcWeightArrayFillCnt < adcWeightArraySize)
        ++adcWeightArrayFillCnt;
}