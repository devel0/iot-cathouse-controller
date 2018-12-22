#include "Config.h"

int portToPin(int pin)
{
    switch (pin)
    {
    case 1:
        return MOSFET_P1;
        break;
    case 2:
        return MOSFET_P2;
        break;
    case 3:
        return MOSFET_P3;
        break;
    case 4:
        return MOSFET_P4;
        break;
    case 5:
        return LED_PIN;
        break;
    case 6:
        return FAN_PIN;
        break;
    }

    return LED_PIN;
}