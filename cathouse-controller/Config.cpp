#include "Config.h"
#include "EEUtil.h"

Config config;

void saveConfig()
{
    EEWrite(&config);
    Serial.println("config saved");
}

void loadConfig()
{
    EERead(&config);
    Serial.println("config laoded");
}

int heatPortIndexToPin(int idx)
{
    switch (idx)
    {
    case 0:
        return MOSFET_P1;
        break;
    case 1:
        return MOSFET_P2;
        break;
    case 2:
        return MOSFET_P3;
        break;
    case 3:
        return MOSFET_P4;
        break;
    }

    return MOSFET_P1;
}