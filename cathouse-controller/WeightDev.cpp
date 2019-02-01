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

void printAdcInfo()
{
    Serial.printf("engine>   adcWeightArrayOff = %d\n", adcWeightArrayOff);
}

bool catExitedStarted = false;
unsigned long catExitedStart;

void EvalAdcWeight()
{
    if (adcWeightArray == NULL)
    {
        adcWeightArraySize = ADCWEIGHT_HISTORY_BACKLOG_KB * 1024 / sizeof(uint16_t);
        adcWeightArray = (uint16_t *)malloc(sizeof(uint16_t) * adcWeightArraySize);
    }

    adcWeight = analogRead(ADCWEIGHT_PIN);
    auto enabled_ports_count = getPorts();

    if (adcWeightArrayOff == adcWeightArraySize)
        adcWeightArrayOff = 0;

    adcWeightArray[adcWeightArrayOff++] = adcWeight;

    if (adcWeightArrayFillCnt < adcWeightArraySize)
        ++adcWeightArrayFillCnt;

    if (adcWeight >= eeJsonConfig.adcWeightMeanCatInMinimum)
    {
        catExitedStarted = false;

        if (!catInThere)
        {
            Serial.printf("engine> cat entered because weight >= %f\n",
                          eeJsonConfig.adcWeightMeanCatInMinimum);
            catInThere = true;
            // if cat in there overriden we waited cat entered truly allow deactivation
            catInThereOverriden = false;
            printAdcInfo();
        }
    }
    else if (!catInThereOverriden)
    {        
        if (!catExitedStarted)
        {
            catExitedStarted = true;
            catExitedStart = millis();
        }

        if (catInThere && timeDiff(catExitedStart, millis()) >= ((unsigned long)eeJsonConfig.catExitThresholdMin) * 60 * 1000)
        {
            Serial.printf("engine> cat exited because weight < %f for more than %d minutes\n",
                          eeJsonConfig.adcWeightMeanCatInMinimum,
                          eeJsonConfig.catExitThresholdMin);
            catInThere = false;
            printAdcInfo();
        }
    }
}