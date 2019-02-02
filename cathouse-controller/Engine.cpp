#include "WeightDev.h"
#include "Util.h"
#include "Config.h"
#include "EEJsonConfig.h"
#include "TempDev.h"
#include "Engine.h"

// catInThere managed by WeightDev EvalAdcWeight()

CycleTypes prevCycle = none;
unsigned long prevCycleBegin = 0;

CycleTypes currentCycle = none;
unsigned long currentCycleBegin = millis();

unsigned long lastEngineProcessExec = millis();

bool catInThereOverriden = false;

// retrieve port in order of preference from 1 to 4
int getPortPref(int pref)
{
    switch (pref)
    {
    case 1:
        return PORT_PREF_1;
    case 2:
        return PORT_PREF_2;
    case 3:
        return PORT_PREF_3;
    case 4:
        return PORT_PREF_4;
    }
}

// if 0 turn off all ports
// if 4 turn on all ports
// if >0 turn on quantity ports from list in order of preference
void setPorts(int quantity)
{
    for (int px = 1; px <= 4; ++px)
    {
        digitalWrite(getPortPref(px), (quantity > 0) ? HIGH : LOW);
        --quantity;
    }
}

// retrieve the nr. of enabled ports
int getPorts()
{
    int res = 0;

    if (digitalRead(MOSFET_P1) == HIGH)
        ++res;
    if (digitalRead(MOSFET_P2) == HIGH)
        ++res;
    if (digitalRead(MOSFET_P3) == HIGH)
        ++res;
    if (digitalRead(MOSFET_P4) == HIGH)
        ++res;

    return res;
}

void fanOn()
{
    digitalWrite(FAN_PIN, HIGH);
}

void fanOff()
{
    digitalWrite(FAN_PIN, LOW);
}

String getCycleStr(CycleTypes cycle)
{
    switch (cycle)
    {
    case none:
        return "none";
    case active:
        return "active";
    case cooldown:
        return "cooldown";
    case disabled:
        return "disabled";
    case manual:
        return "manual";
    default:
        return "unknown";
    }
}

// save port for manual mode (if cooldown occurs)
int savedPorts[6];
void savePorts()
{
    savedPorts[0] = digitalRead(MOSFET_P1);
    savedPorts[1] = digitalRead(MOSFET_P2);
    savedPorts[2] = digitalRead(MOSFET_P3);
    savedPorts[3] = digitalRead(MOSFET_P4);
    savedPorts[4] = digitalRead(LED_PIN);
    savedPorts[5] = digitalRead(FAN_PIN);
}

// restore port for manual mode (if cooldown occured)
void restorePorts()
{
    digitalWrite(MOSFET_P1, savedPorts[0]);
    digitalWrite(MOSFET_P2, savedPorts[1]);
    digitalWrite(MOSFET_P3, savedPorts[2]);
    digitalWrite(MOSFET_P4, savedPorts[3]);
    digitalWrite(LED_PIN, savedPorts[4]);
    digitalWrite(FAN_PIN, savedPorts[5]);
}

void setCurrentCycle(CycleTypes cycle)
{
    prevCycleBegin = currentCycleBegin;
    prevCycle = currentCycle;
    currentCycleBegin = millis();
    currentCycle = cycle;
}

void setManual()
{
    Serial.printf("engine> setManual");
    savePorts();
    setCurrentCycle(manual);
}

void unsetManual()
{
    Serial.printf("engine> unsetManual");
    setCurrentCycle(none);
}

void backPrevCycle()
{
    auto cb = currentCycleBegin;
    auto c = currentCycle;
    currentCycle = prevCycle;
    currentCycleBegin = prevCycleBegin;
    prevCycle = c;
    prevCycleBegin = cb;
}

unsigned long lastStandbyTrendEval = millis();
unsigned long lastFanActivation = millis();

void engineProcess()
{
    if (timeDiff(lastEngineProcessExec, millis()) < ENGINE_POOL_INTERVAL_MS)
        return;

    if (eeJsonConfig.manualMode && currentCycle != manual)
        setManual();

    // disable ports if textern exceed
    if (!eeJsonConfig.manualMode && textern_assigned && textern >= eeJsonConfig.texternGTESysOff)
    {
        if (currentCycle != disabled)
            setCurrentCycle(disabled);

        setPorts(0);

        Serial.printf("engine> disabled because textern %f >= %f\n", textern, eeJsonConfig.texternGTESysOff);
    }

    // back from disabled to prev
    if (currentCycle == disabled && timeDiff(currentCycleBegin, millis()) > 60000L)
    {
        backPrevCycle();
    }

    // cooldown if one of tbottom,twood,tambient exceed
    if ((tbottom_assigned && tbottom >= eeJsonConfig.tbottomLimit) ||
        (twood_assigned && twood >= eeJsonConfig.twoodLimit) ||
        (tambient_assigned && tambient >= eeJsonConfig.tambientLimit))
    {
        if (currentCycle != cooldown && currentCycle != none)
        {
            setCurrentCycle(cooldown);
            Serial.printf("engine> cooldown because tbottom %f>=%f or twood %f>=%f or tambient %f>=%f\n",
                          tbottom, eeJsonConfig.tbottomLimit,
                          twood, eeJsonConfig.twoodLimit,
                          tambient, eeJsonConfig.tambientLimit);
        }

        setPorts(0);
    }

    // back from cooldown to prev
    if (currentCycle == cooldown && timeDiff(currentCycleBegin, millis()) > eeJsonConfig.cooldownTimeMs)
    {
        backPrevCycle();
    }

    // automatic mode
    if (currentCycle != manual)
    {
        if (!catInThere)
        {
            if (currentCycle != none)
                setCurrentCycle(none);
            setPorts(0);
            digitalWrite(FAN_PIN, LOW);
        }
        else // CAT IS IN THERE
        {
            // initiate active cycle
            if (currentCycle != disabled && currentCycle != cooldown &&
                currentCycle != active)
            {
                setCurrentCycle(active);
            }

            // manage fan
            {
                if (tbottom_assigned && twood_assigned)
                {
                    auto pfan = digitalRead(FAN_PIN);

                    if (pfan == LOW &&
                        (tbottom >= eeJsonConfig.tbottomGTEFanOn || twood >= eeJsonConfig.twoodGTEFanOn))
                    {
                        lastFanActivation = millis();
                        digitalWrite(FAN_PIN, HIGH);
                        Serial.printf("engine> enable fan because tbottom %f>=%f or twood %f>=%f\n",
                                      tbottom, eeJsonConfig.tbottomGTEFanOn,
                                      twood, eeJsonConfig.twoodGTEFanOn);
                    }
                    else if (pfan == HIGH &&
                             timeDiff(lastFanActivation, millis()) >= ((unsigned long)ENGINE_FAN_DEACTIVATION_THRESHOLD_MIN) * 1000 * 60 &&
                             tbottom < eeJsonConfig.tbottomGTEFanOn && twood < eeJsonConfig.twoodGTEFanOn)
                    {
                        digitalWrite(FAN_PIN, LOW);
                        Serial.printf("engine> disable fan\n");
                    }
                }
            }

            switch (currentCycle)
            {

            case active:
            {

                if (tbottom_assigned && twood_assigned)
                {
                    auto diffBottom = tbottom - (eeJsonConfig.tbottomLimit - eeJsonConfig.targetTempFromLimit);
                    auto diffWood = twood - (eeJsonConfig.twoodLimit - eeJsonConfig.targetTempFromLimit);

                    if (timeDiff(lastStandbyTrendEval, millis()) > ENGINE_STANDBY_REACTION_INTERVAL_MS)
                    {
                        auto ports = getPorts();
                        auto portsToSet = -1;

                        if (max(diffBottom, diffWood) > -TBOTTOM_TREND_DELTA_C)
                        {
                            if (ports > 0)
                            {
                                portsToSet = 0;
                            }
                        }
                        else
                        {
                            auto distance = -max(diffBottom, diffWood);

                            int pq = (int)(min(distance / TBOTTOM_TREND_DELTA_C, 4.0));
                            if (pq != ports)
                            {
                                portsToSet = pq;
                            }
                        }
                        if (portsToSet != -1)
                        {
                            setPorts(portsToSet);
                            Serial.printf("engine> tbottom=%f (target=%f) and twood=%f (target=%f) makes %d ports active\n",
                                          tbottom, eeJsonConfig.tbottomLimit - eeJsonConfig.targetTempFromLimit,
                                          twood, eeJsonConfig.twoodLimit - eeJsonConfig.targetTempFromLimit,
                                          portsToSet);
                        }

                        lastStandbyTrendEval = millis();
                    }
                    break;
                }
            }
            }

            lastEngineProcessExec = millis();
        }
    }
}