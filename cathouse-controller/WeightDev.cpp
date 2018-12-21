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

int adcWeightArrayOffLastEvent = 0;

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

    auto s = 0; // distance of current off from last event off
    if (adcWeightArrayOff >= adcWeightArrayOffLastEvent)
        s = adcWeightArrayOff - adcWeightArrayOffLastEvent;
    else
        s = adcWeightArraySize - adcWeightArrayOffLastEvent + adcWeightArrayOff;    
    if (adcWeightArrayFillCnt > 20)
    {
        auto meanLast10Samples = 0.0;
        auto j = adcWeightArrayOff < 10 ? (adcWeightArrayFillCnt - (10 - adcWeightArrayOff)) : (adcWeightArrayOff - 10);
        for (int i = 0; i < 10; ++i)
        {
            meanLast10Samples += adcWeightArray[j++];
            if (j >= adcWeightArraySize)
                j = 0;
        }
        meanLast10Samples /= 10.0;

        auto meanFromLastEvent = 0.0;
        j = adcWeightArrayOffLastEvent;

        for (int i = 0; i < s - 10; ++i)
        {
            meanFromLastEvent += adcWeightArray[j++];
            if (j >= adcWeightArraySize)
                j = 0;
        }
        meanFromLastEvent /= (s - 10);

        if (s > 10)
        {
            if (meanFromLastEvent > meanLast10Samples)
            {
                if (meanFromLastEvent - meanLast10Samples >= eeJsonConfig.adcWeightDeltaCat)
                {
                    catInThere = false;
                    adcWeightArrayOffLastEvent = adcWeightArrayOff;                    
                }
            }
            else
            {
                if (meanLast10Samples - meanFromLastEvent >= eeJsonConfig.adcWeightDeltaCat)
                {
                    catInThere = true;
                    adcWeightArrayOffLastEvent = adcWeightArrayOff;                    
                }
            }
        }        
    }
}