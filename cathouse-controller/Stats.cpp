#include "Stats.h"
#include "Util.h"
#include "Config.h"
#include "EEJsonConfig.h"
#include "WeightDev.h"

double runtime_hr = 0.0;
double Wh = 0.0;

uint32_t freeram_min = 80 * 1024, freeram = 80 * 1024;
// used for consumption stat
bool p_was_active[4];

unsigned long lastStatsUpdate = millis();

void statsInit()
{
    for (int i = 0; i < 4; ++i)
        p_was_active[i] = false;
}

void statsUpdate()
{
    auto tdelta = timeDiff(lastStatsUpdate, millis());
    if (tdelta < UPDATE_STATS_INTERVAL_MS)
        return;

    {

        EvalAdcWeight();
    }

    {
        freeram = freeMemorySum();
        freeram_min = min(freeram, freeram_min);
    }

    {
        auto hr = ((double)tdelta) / ((double)1000.0 * 60 * 60);

        for (int i = 0; i < 4; ++i)
        {
            auto p = portToPin(i + 1);
            auto v = digitalRead(p);
            if (v == HIGH && p_was_active[i])
            {
                Wh += W_PORT * hr;
            }
            p_was_active[i] = v == HIGH;
        }

        runtime_hr += hr;
    }

    lastStatsUpdate = millis();
}