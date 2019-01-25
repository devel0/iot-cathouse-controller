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

double meanFromLastEvent = 0.0;
double meanLastSamples = 0.0;

void printAdcInfo()
{
    Serial.printf("engine>   adcWeightArrayOffLastEvent = %d\n", adcWeightArrayOffLastEvent);
    Serial.printf("engine>   meanFromLastEvent = %f\n", meanFromLastEvent);
    Serial.printf("engine>   adcWeightArrayOff = %d\n", adcWeightArrayOff);
    Serial.printf("engine>   meanLastSamples = %f\n", meanLastSamples);
}

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
    if (adcWeightArrayFillCnt > ADCWEIGHT_LASTSAMPLES_CNT * 2)
    {
        meanLastSamples = 0.0;
        auto j = 0;
        if (adcWeightArrayOff < ADCWEIGHT_LASTSAMPLES_CNT)
            j = adcWeightArrayFillCnt - (ADCWEIGHT_LASTSAMPLES_CNT - adcWeightArrayOff);
        else
            j = adcWeightArrayOff - ADCWEIGHT_LASTSAMPLES_CNT;
        for (int i = 0; i < ADCWEIGHT_LASTSAMPLES_CNT; ++i)
        {
            meanLastSamples += adcWeightArray[j++];
            if (j >= adcWeightArraySize)
                j = 0;
        }
        meanLastSamples /= ADCWEIGHT_LASTSAMPLES_CNT;

        meanFromLastEvent = 0.0;
        j = adcWeightArrayOffLastEvent;

        for (int i = 0; i < s - ADCWEIGHT_LASTSAMPLES_CNT; ++i)
        {
            meanFromLastEvent += adcWeightArray[j++];
            if (j >= adcWeightArraySize)
                j = 0;
        }
        meanFromLastEvent /= (s - ADCWEIGHT_LASTSAMPLES_CNT);

        if (s > 2 * ADCWEIGHT_LASTSAMPLES_CNT)
        {
            if (meanFromLastEvent > meanLastSamples)
            {
                if (meanFromLastEvent - meanLastSamples >= eeJsonConfig.adcWeightDeltaCat)
                {
                    Serial.printf("engine> cat exited because %f-%f>=%d\n", meanFromLastEvent, meanLastSamples, eeJsonConfig.adcWeightDeltaCat);
                    catInThere = false;
                    printAdcInfo();
                    adcWeightArrayOffLastEvent = adcWeightArrayOff;
                }
            }
            else
            {
                if (meanLastSamples - meanFromLastEvent >= eeJsonConfig.adcWeightDeltaCat)
                {
                    Serial.printf("engine> cat entered because %f-%f>=%d\n", meanLastSamples, meanFromLastEvent, eeJsonConfig.adcWeightDeltaCat);
                    catInThere = true;
                    printAdcInfo();
                    adcWeightArrayOffLastEvent = adcWeightArrayOff;
                }
            }
        }
    }
}