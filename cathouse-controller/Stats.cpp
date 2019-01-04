#include "Stats.h"
#include "Util.h"
#include "Config.h"
#include "EEJsonConfig.h"
#include "WeightDev.h"
#include "TempDev.h"
#include "Engine.h"

double runtime_hr = INITIAL_EMPTY_RUNTIME_HR;
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

void updateHistory()
{
    if (temperatureHistory != NULL &&
        (timeDiff(lastTemperatureHistoryRecord, millis()) > 1000UL * temperatureHistoryIntervalSec))
    {
        if (temperatureHistoryFillCnt < temperatureHistorySize)
            ++temperatureHistoryFillCnt;

        if (temperatureHistoryOff == temperatureHistorySize)
            temperatureHistoryOff = 0;

        for (int i = 0; i < temperatureDeviceCount; ++i)
            temperatureHistory[i][temperatureHistoryOff] = temperatures[i];

        catInThereHistory->Set(temperatureHistoryOff, catInThere);
        p1History->Set(temperatureHistoryOff, digitalRead(MOSFET_P1) == HIGH);
        p2History->Set(temperatureHistoryOff, digitalRead(MOSFET_P2) == HIGH);
        p3History->Set(temperatureHistoryOff, digitalRead(MOSFET_P3) == HIGH);
        p4History->Set(temperatureHistoryOff, digitalRead(MOSFET_P4) == HIGH);
        fanHistory->Set(temperatureHistoryOff, digitalRead(FAN_PIN) == HIGH);
        disabledHistory->Set(temperatureHistoryOff, currentCycle == disabled);
        cooldownHistory->Set(temperatureHistoryOff, currentCycle == cooldown);

        ++temperatureHistoryOff;
        lastTemperatureHistoryRecord = millis();
    }

    lastStatsUpdate = millis();
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

    updateHistory();
}