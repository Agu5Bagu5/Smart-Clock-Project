#include <Arduino.h>
#include "blinkLogic.h"

bool blinkLogic(int interval)
{
    static unsigned long last = 0;

    if (millis() - last >= interval)
    {
        last = millis();
        return true;
    }

    return false;
}