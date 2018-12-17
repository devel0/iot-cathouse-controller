#include "Config.h"

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