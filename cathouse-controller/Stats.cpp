#include "Stats.h"
#include "Util.h"
#include "Config.h"
#include "EEJsonConfig.h"

double runtime_hr = 0.0;
double Wh = 0.0;
extern unsigned long freeram_lastupdate = millis();
extern uint32_t freeram_min = 80 * 1024, freeram = 80 * 1024;
// used for consumption stat
bool p_was_active[4];
unsigned long lastConsumptionUpdate = millis();

void statsInit()
{
    for (int i = 0; i < 4; ++i)
        p_was_active[i] = false;
}

void statsUpdate()
{
    {
        auto tdelta = timeDiff(freeram_lastupdate, millis());
        if (tdelta >= eeJsonConfig.updateFreeramIntervalMs && tdelta >= FREERAM_UPDATE_INTERVAL_MIN_MS)
        {
            freeram = freeMemorySum();
            freeram_min = min(freeram, freeram_min);
            freeram_lastupdate = millis();
        }
    }

    {
        auto tdelta = timeDiff(lastConsumptionUpdate, millis());
        if (tdelta >= eeJsonConfig.updateConsumptionIntervalMs && tdelta >= CONSUMPTION_UPDATE_INTERVAL_MIN_MS)
        {
            auto hr = ((double)tdelta) / ((double)1000.0 * 60 * 60);

            for (int i = 0; i < 4; ++i)
            {
                auto p = heatPortIndexToPin(i);
                auto v = digitalRead(p);
                if (v == HIGH && p_was_active[i])
                {
                    Wh += W_PORT * hr;
                }
                p_was_active[i] = v == HIGH;
            }

            runtime_hr += hr;
            lastConsumptionUpdate = millis();
        }
    }
}