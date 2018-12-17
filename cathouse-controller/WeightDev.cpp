#include "WeightDev.h"
#include "Util.h"

unsigned long lastWeightTimestamp = millis();

const int MEAN_CNT = 10;
double meanWeight = 0.0;
int weightLast10Idx = 0;
int weightLast10[MEAN_CNT];
bool meanWeightSetReady = false;

void manageWeight()
{
    if (timeDiff(lastWeightTimestamp, millis()) > 500)
    {
        auto v = analogRead(A0);
        Serial.printf("A0 = %d ", v);
      /*  if (meanWeightSetReady)
        {
            if (weightLast10Idx == 10)
                weightLast10Idx = 0;
            weightLast10[weightLast10Idx++] = v;
            meanWeight = 0;
            for (int i = 0; i < MEAN_CNT; ++i)
            {
                Serial.printf(" %d", weightLast10[i]);
                meanWeight += weightLast10[i];
            }
            meanWeight /= MEAN_CNT;

            Serial.printf(" [ %f ]", meanWeight);
        }
        else
        {
            weightLast10[weightLast10Idx++] = v;
            if (weightLast10Idx == 10)
                meanWeightSetReady = true;
        }*/
        Serial.printf("\n");

        lastWeightTimestamp = millis();
    }
}