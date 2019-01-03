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

bool fanlessStarted = false;
int fanlessCurrentPort = 1;
unsigned long fanlessCurrentPortBegin = 0;
int fanlessNextPort = 2;
unsigned long fanlessNextPortBegin = 0;

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
    case fanless:
        return "fanless";
    default:
        return "unknown";
    }
}

void engineProcess()
{
    if (timeDiff(lastEngineProcessExec, millis()) < ENGINE_POOL_INTERVAL_MS)
        return;

    if (eeJsonConfig.manualMode)
    {
        lastEngineProcessExec = millis();
        return;
    }

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

            Serial.printf("engine[%ld]> disabled because textern %f >= %f\n", millis() / 1000, textern, eeJsonConfig.texternGTESysOff);
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

                    Serial.printf("engine[%ld]> enabled because textern %f < %f\n", millis() / 1000, textern, eeJsonConfig.texternGTESysOff);
                }
            }
            break;
            case fanless:
            case fullpower:
            case standby:
            {
                if ((tbottom_assigned && tbottom >= eeJsonConfig.tbottomLimit) ||
                    (twood_assigned && twood >= eeJsonConfig.twoodLimit) ||
                    (tambient_assigned && tambient >= eeJsonConfig.tambientLimit))
                {
                    prevCycle = currentCycle;
                    prevCycleBegin = currentCycleBegin;
                    currentCycle = cooldown;
                    currentCycleBegin = millis();

                    setDisablePorts();

                    Serial.printf("engine[%ld]> cooldown because tbottom %f>=%f or twood %f>=%f or tambient %f>=%f\n",
                                  millis() / 1000,
                                  tbottom, eeJsonConfig.tbottomLimit,
                                  twood, eeJsonConfig.twoodLimit,
                                  tambient, eeJsonConfig.tambientLimit);
                }
            }
            break;
            case cooldown:
            {
                if (timeDiff(currentCycleBegin, millis()) > eeJsonConfig.cooldownTimeMs)
                {
                    auto cooldownBegin = currentCycleBegin;

                    currentCycle = prevCycle;
                    currentCycleBegin = prevCycleBegin;
                    prevCycle = cooldown;
                    prevCycleBegin = cooldownBegin;                    

                    Serial.printf("engine[%ld]> resume previous cycle [%s] from cooldown because cooldown time %lu min expired\n",
                                  millis() / 1000,
                                  getCycleStr(prevCycle).c_str(),
                                  eeJsonConfig.cooldownTimeMs / 1000 / 60);
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
            if (eeJsonConfig.fanlessMode)
            {
                fanlessCurrentPort = 1;
                fanlessCurrentPortBegin = millis();

                currentCycle = fanless;
                currentCycleBegin = millis();
            }
            else
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

                    Serial.printf("engine[%ld]> back to fullpower from off cycle because prev cycle expired and cat is in there\n",
                                  millis() / 1000);
                }
                else
                {
                    currentCycle = prevCycle;
                    currentCycleBegin = prevCycleBegin;

                    switch (prevCycle)
                    {
                    case fullpower:
                    {
                        setFullpowerPorts();
                        Serial.printf("engine[%ld]> back to fullpower from off cycle because prev cycle still valid and cat is in there\n",
                                      millis() / 1000);
                    }
                    break;

                    case standby:
                    {
                        setStandbyPorts();
                        Serial.printf("engine[%ld]> back to standby from off cycle because prev cycle still valid and cat is in there\n",
                                      millis() / 1000);
                    }
                    break;
                    }
                }
            }
        }
        break;
        case fanless:
        {
            auto currentPortAge = timeDiff(fanlessCurrentPortBegin, millis());
            if (!fanlessStarted)
            {
                fanlessCurrentPort = 1;
                fanlessCurrentPortBegin = millis();
                digitalWrite(portToPin(fanlessCurrentPort), HIGH);
                fanlessStarted = true;
                Serial.printf("engine[%ld]> initiate fanless port=%d\n", millis() / 1000, fanlessCurrentPort);
            }
            // if current port expired
            else if (currentPortAge > eeJsonConfig.portDurationMs)
            {
                // if next port already started make it current
                if (fanlessNextPortBegin > fanlessCurrentPortBegin)
                {
                    Serial.printf("engine[%ld]> switch from port=%d to %d\n", millis() / 1000, fanlessCurrentPort, fanlessNextPort);

                    digitalWrite(portToPin(fanlessCurrentPort), LOW);
                    fanlessCurrentPort = fanlessNextPort;
                    fanlessCurrentPortBegin = fanlessNextPortBegin;
                    digitalWrite(portToPin(fanlessCurrentPort), HIGH);
                }
                else // elsewhere start it
                {
                    digitalWrite(portToPin(fanlessCurrentPort), LOW);
                    ++fanlessCurrentPort;
                    if (fanlessCurrentPort > 4)
                        fanlessCurrentPort = 1;
                    fanlessCurrentPortBegin = millis();
                    digitalWrite(portToPin(fanlessCurrentPort), HIGH);

                    Serial.printf("engine[%ld]> current port is now %d\n", millis() / 1000, fanlessCurrentPort);
                }
            }
            // elsewhere checks for nextport start
            else if (eeJsonConfig.portOverlapDurationMs > 0 && fanlessNextPortBegin <= fanlessCurrentPortBegin)
            {
                if (timeDiff(fanlessCurrentPortBegin, millis()) > eeJsonConfig.portDurationMs - eeJsonConfig.portOverlapDurationMs)
                {
                    fanlessNextPort = fanlessCurrentPort + 1;
                    if (fanlessNextPort > 4)
                        fanlessNextPort = 1;
                    fanlessNextPortBegin = millis();
                    digitalWrite(portToPin(fanlessNextPort), HIGH);

                    Serial.printf("engine[%ld]> starts nextport %d\n", millis() / 1000, fanlessNextPort);
                }
            }
        }
        break;

        case fullpower:
        {
            auto currentCycleDuration = timeDiff(currentCycleBegin, millis());

            auto pfan = digitalRead(FAN_PIN);
            // turn fan on when tbottom reach enough temp
            if (tbottom_assigned)
            {

                if (tbottom >= eeJsonConfig.tbottomGTEFanOn && pfan == LOW)
                {
                    digitalWrite(FAN_PIN, HIGH);
                    Serial.printf("engine[%ld]> enable fan because tbottom %f>=%f in fullpower cycle\n",
                                  millis() / 1000,
                                  tbottom, eeJsonConfig.tbottomGTEFanOn);
                }
                else if (tbottom < eeJsonConfig.tbottomGTEFanOn && pfan == HIGH)
                {
                    digitalWrite(FAN_PIN, LOW);
                    Serial.printf("engine[%ld]> disable fan because tbottom %f<%f in fullpower cycle\n",
                                  millis() / 1000,
                                  tbottom, eeJsonConfig.tbottomGTEFanOn);
                }
            }
            else // fallback if not assigned id to temp device to detect bottom enable fan in fullpower
            {
                if (pfan == LOW)
                {
                    digitalWrite(FAN_PIN, HIGH);
                    Serial.printf("engine[%ld]> enable fan because tbottom can't detect due to unassigned temp device id in fullpower cycle\n",
                                  millis() / 1000);
                    Serial.printf("engine>   tbottom_id = [%s]\n", eeStaticConfig.tbottomId);
                    Serial.printf("engine>   available ids:\n");
                    for (int i = 0; i < temperatureDeviceCount; ++i)
                    {
                        auto id = tempDevHexAddress[i];
                        Serial.printf("engine>   [%s]\n", id);
                    }
                }
            }

            // autoswitch next cycle when this expired
            if (currentCycleDuration >= eeJsonConfig.fullpowerDurationMs)
            {
                currentCycle = standby;
                currentCycleBegin = millis();
                setStandbyPorts();

                Serial.printf("engine[%ld]> switch to standby cycle because fullpower expired\n", millis() / 1000);
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

                Serial.printf("engine[%ld]> switch to fullpower cycle because standby expired\n", millis() / 1000);
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
        case fanless:
        {
            prevCycle = currentCycle;
            prevCycleBegin = currentCycleBegin;

            currentCycle = none;
            currentCycleBegin = millis();

            Serial.printf("engine[%ld]> goes off because cat exited\n", millis() / 1000);
        }
        break;
        }
        setDisablePorts();
    }

    lastEngineProcessExec = millis();
}
