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
    case fullpower:
        return "fullpower";
    case standby:
        return "standby";
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

enum TTrendTypes
{
    increasing,
    stable,
    decreasing
};
TTrendTypes getTBottomTrend()
{
    auto diff = tbottom - (eeJsonConfig.tbottomLimit - ENGINE_STANDBY_TARGET_TEMP_DISTANCE);

    if (diff > TBOTTOM_TREND_DELTA_C)
        return increasing;
    else if (diff < -TBOTTOM_TREND_DELTA_C)
        return decreasing;

    return stable;
}

unsigned long lastStandbyTrendEval = millis();

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
        if (currentCycle != cooldown)
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
            // initiate fullpower
            if (currentCycle != disabled && currentCycle != cooldown &&
                currentCycle != fullpower && currentCycle != standby)
            {
                setCurrentCycle(fullpower);
            }

            switch (currentCycle)
            {
            case fullpower:
            {
                if (tbottom_assigned)
                {
                    // manage fan
                    auto pfan = digitalRead(FAN_PIN);
                    if (tbottom >= eeJsonConfig.tbottomGTEFanOn && pfan == LOW)
                    {
                        digitalWrite(FAN_PIN, HIGH);
                        Serial.printf("engine> enable fan because tbottom %f>=%f in fullpower cycle\n",
                                      tbottom, eeJsonConfig.tbottomGTEFanOn);
                    }
                    else if (tbottom < eeJsonConfig.tbottomGTEFanOn - 1 && pfan == HIGH)
                    {
                        digitalWrite(FAN_PIN, LOW);
                        Serial.printf("engine> disable fan because tbottom %f<%f-1 in fullpower cycle\n",
                                      tbottom, eeJsonConfig.tbottomGTEFanOn);
                    }

                    if (tbottom >= eeJsonConfig.twoodGTEFanOn && pfan == LOW)
                    {
                        digitalWrite(FAN_PIN, HIGH);
                        Serial.printf("engine> enable fan because twood %f>=%f\n",
                                      tbottom, eeJsonConfig.twoodGTEFanOn);
                    }
                    else if (tbottom < eeJsonConfig.twoodGTEFanOn - 1 && pfan == HIGH)
                    {
                        digitalWrite(FAN_PIN, LOW);
                        Serial.printf("engine> disable fan because twood %f<%f-1 in fullpower cycle\n",
                                      tbottom, eeJsonConfig.twoodGTEFanOn);
                    }

                    auto tFromLimit = eeJsonConfig.tbottomLimit - tbottom;
                    if (tFromLimit <= ENGINE_STANDBY_TARGET_TEMP_DISTANCE)
                    {
                        Serial.printf("engine> switch to standby cycle disabling fan if any\n");
                        setCurrentCycle(standby);
                        setPorts(2);
                        digitalWrite(FAN_PIN, LOW);
                    }
                    else
                    {
                        if (getPorts() != 4)
                        {
                            Serial.printf("engine> enable fullpower cycle ports\n");
                            setPorts(4);
                        }
                    }
                }
            }
            break;

            case standby:
            {
                if (tbottom_assigned)
                {
                    auto trend = getTBottomTrend();

                    if (timeDiff(lastStandbyTrendEval, millis()) > ENGINE_STANDBY_REACTION_INTERVAL_MS)
                    {
                        auto ports = getPorts();
                        switch (trend)
                        {
                        case increasing:
                        {
                            if (ports > 0)
                            {
                                setPorts(ports - 1);
                                Serial.printf("engine> set %d ports quantity due to increasing tbottom trend\n", ports - 1);
                            }
                        }
                        break;
                        case decreasing:
                        {
                            if (ports < 4)
                            {
                                setPorts(ports + 1);
                                Serial.printf("engine> set %d ports quantity due to decreasing tbottom trend\n", ports + 1);
                            }
                        }
                        break;
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