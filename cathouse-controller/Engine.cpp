#include "WeightDev.h"
#include "Util.h"
#include "Config.h"
#include "EEJsonConfig.h"
#include "TempDev.h"

// catInThere managed by WeightDev EvalAdcWeight()

enum CycleTypes
{
    none,
    fullpower,
    standby,
    cooldown,
    disabled
};

CycleTypes prevCycle = none;
unsigned long prevCycleBegin = 0;

CycleTypes currentCycle = none;
unsigned long currentCycleBegin = millis();

unsigned long lastEngineProcessExec = millis();

void setFullpowerPorts()
{
    digitalWrite(MOSFET_P1, HIGH);
    digitalWrite(MOSFET_P2, HIGH);
    digitalWrite(MOSFET_P3, HIGH);
    digitalWrite(MOSFET_P4, HIGH);
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
}

void setStandbyPorts()
{
    digitalWrite(MOSFET_P1, (eeJsonConfig.standbyPort == 1) ? HIGH : LOW);
    digitalWrite(MOSFET_P2, (eeJsonConfig.standbyPort == 2) ? HIGH : LOW);
    digitalWrite(MOSFET_P3, (eeJsonConfig.standbyPort == 3) ? HIGH : LOW);
    digitalWrite(MOSFET_P4, (eeJsonConfig.standbyPort == 4) ? HIGH : LOW);
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
}

void setDisablePorts()
{
    digitalWrite(MOSFET_P1, LOW);
    digitalWrite(MOSFET_P2, LOW);
    digitalWrite(MOSFET_P3, LOW);
    digitalWrite(MOSFET_P4, LOW);
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
}

void engineProcess()
{
    if (timeDiff(lastEngineProcessExec, millis()) < ENGINE_POOL_INTERVAL_MS)
        return;

    if (catInThere) // on
    {

        //
        // manage special cases
        //
        if (textern_assigned && textern >= eeJsonConfig.texternGTESysOff)
        {
            currentCycle = prevCycle = disabled;
            currentCycleBegin = millis();
            setDisablePorts();
        }
        else
        {
            switch (currentCycle)
            {
            case disabled:
            {
                if (textern_assigned && textern < eeJsonConfig.texternGTESysOff)
                {
                    prevCycle = none;
                    currentCycle = none;
                    currentCycleBegin = millis();
                }
            }
            break;
            case fullpower:
            case standby:
            {
                if ((tbottom_assigned && tbottom >= eeJsonConfig.tbottomLimit) ||
                    (twood_assigned && twood >= eeJsonConfig.twoodLimit) ||
                    (tambient_assigned && tambient >= eeJsonConfig.tambientId))
                {
                    prevCycle = currentCycle;
                    prevCycleBegin = currentCycleBegin;
                    currentCycle = cooldown;
                    currentCycleBegin = millis();
                }
            }
            break;
            case cooldown:
            {
                if (timeDiff(currentCycleBegin, millis()) > eeJsonConfig.cooldownTimeMs)
                {
                    let cooldownBegin = currentCycleBegin;

                    currentCycle = prevCycle;
                    currentCycleBegin = prevCycleBegin;
                    prevCycle = cooldown;
                    prevCycleBegin = cooldownBegin;
                }
            }
            break;
            }
        }

        //
        // inititate fullpower cycle or switch between standby-fullpower modes
        //
        switch (currentCycle)
        {
        case none:
        {
            auto prevCycleExpired = true;

            if (prevCycle == fullpower && timeDiff(prevCycleBegin, millis()) < eeJsonConfig.fullpowerDurationMs)
                prevCycleExpired = false;

            else if (prevCycle == standby && timeDiff(prevCycleBegin, millis()) < eeJsonConfig.standbyDurationMs)
                prevCycleExpired = false;

            if (prevCycleExpired)
            {
                currentCycle = fullpower;
                currentCycleBegin = millis();

                setFullpowerPorts();
            }
            else
            {
                currentCycle = prevCycle;
                currentCycleBegin = prevCycleBegin;

                switch (prevCycle)
                {
                case fullpower:
                    setFullpowerPorts();
                    break;

                case standby:
                    setStandbyPorts();
                    break;
                }
            }
        }
        break;

        case fullpower:
        {
            auto currentCycleDuration = timeDiff(currentCycleBegin, millis());

            // turn fan on when tbottom reach enough temp
            if (tbottom_assigned)
            {
                if (tbottom >= eeJsonConfig.tbottomGTEFanOn)
                    digitalWrite(FAN_PIN, HIGH);
                else
                    digitalWrite(FAN_PIN, LOW);
            }
            else // fallback if not assigned id to temp device to detect bottom enable fan in fullpower
            {
                digitalWrite(FAN_PIN, HIGH);
            }

            // autoswitch next cycle when this expired
            if (currentCycleDuration >= eeJsonConfig.fullpowerDurationMs)
            {
                currentCycle = standby;
                currentCycleBegin = millis();
                setStandbyPorts();
            }
        }
        break;

        case standby:
        {
            auto currentCycleDuration = timeDiff(currentCycleBegin, millis());

            // autoswitch next cycle when this expired
            if (currentCycleDuration >= eeJsonConfig.standbyDurationMs)
            {
                currentCycle = fullpower;
                currentCycleBegin = millis();
                setFullpowerPorts();
            }
        }
        break;
        }
    }
    else // off
    {
        //
        // shutdown heat ports and goes in none mode storing in prevCycleBegin and prevCycle previous cycle info
        // for resuming consuming remaining time or restarting from fullpower cycle
        //
        switch (currentCycle)
        {
        case standby:
        case fullpower:
        {
            prevCycle = currentCycle;
            prevCycleBegin = currentCycleBegin;

            currentCycle = none;
            currentCycleBegin = millis();

            digitalWrite(MOSFET_P1, LOW);
            digitalWrite(MOSFET_P2, LOW);
            digitalWrite(MOSFET_P3, LOW);
            digitalWrite(MOSFET_P4, LOW);
            digitalWrite(FAN_PIN, LOW);
            digitalWrite(LED_PIN, LOW);
        }
        break;
        }
    }
}
