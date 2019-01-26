#include "WeightDev.h"
#include "Util.h"
#include "SList.h"
#include "Config.h"
#include "EEJsonConfig.h"
#include "Engine.h"

bool catInThere = false;
uint16_t *adcWeightArray = NULL;
int adcWeightArrayOff = 0;
int adcWeightArraySize = 0;
int adcWeightArrayFillCnt = 0;

double adcWeight = 0.0;

double meanButLastSamples = 0.0;
double meanLastSamples = 0.0;

void printAdcInfo()
{
    Serial.printf("engine>   adcWeightArrayOff = %d\n", adcWeightArrayOff);
    Serial.printf("engine>   meanButLastSamples = %f\n", meanButLastSamples);
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
    auto enabled_ports_count = getPorts();
    auto adcWeightDeltaFullpower = eeJsonConfig.adcWeightDeltaFullpower;
    adcWeight -= (adcWeightDeltaFullpower / 4) * getPorts();

    if (adcWeightArrayOff == adcWeightArraySize)
        adcWeightArrayOff = 0;

    adcWeightArray[adcWeightArrayOff++] = adcWeight;

    if (adcWeightArrayFillCnt < adcWeightArraySize)
        ++adcWeightArrayFillCnt;

    if (adcWeightArrayFillCnt > 2 * ADCWEIGHT_LASTSAMPLES_CNT)
    {
        {
            meanButLastSamples = 0.0;
            auto j = 0;
            if (adcWeightArrayOff < 2 * ADCWEIGHT_LASTSAMPLES_CNT)
                j = adcWeightArrayFillCnt - (2 - ADCWEIGHT_LASTSAMPLES_CNT - adcWeightArrayOff);
            else
                j = adcWeightArrayOff - 2 * ADCWEIGHT_LASTSAMPLES_CNT;
            for (int i = 0; i < ADCWEIGHT_LASTSAMPLES_CNT; ++i)
            {
                meanButLastSamples += adcWeightArray[j++];
                if (j >= adcWeightArraySize)
                    j = 0;
            }
            meanButLastSamples /= ADCWEIGHT_LASTSAMPLES_CNT;
        }

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
        }

        {
            if (!catInThere && meanLastSamples >= meanButLastSamples + eeJsonConfig.adcWeightDeltaCat)
            {
                Serial.printf("engine> cat entered because %f >= %f+%d\n",
                              meanLastSamples,
                              meanButLastSamples,
                              eeJsonConfig.adcWeightDeltaCat);
                catInThere = true;
                printAdcInfo();
            }
            else if (catInThere && meanLastSamples <= meanButLastSamples - eeJsonConfig.adcWeightDeltaCat)
            {
                Serial.printf("engine> cat exited because %f <= %f-%d\n",
                              meanLastSamples,
                              meanButLastSamples,
                              eeJsonConfig.adcWeightDeltaCat);
                catInThere = false;
                printAdcInfo();
            }
        }
    }
}