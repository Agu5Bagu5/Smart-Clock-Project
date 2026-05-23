#include <Arduino.h>
#include "blinkLogic.h"

bool blinkLogic(int interval)
{
    static unsigned long last = 0;
    unsigned long now = millis();

    if (now - last >= interval)
    {
        last = now;
        return true;
    }
    return false;
}