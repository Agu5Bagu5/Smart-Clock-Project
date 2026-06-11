#include <Arduino.h>
#include "constant.h"
#include "buttonPressLogic.h"

// Generic debounced "rising edge" detector used by right/left/remove.
// Returns true once per physical press, then requires release before firing again.
static bool risingEdge(int pin, unsigned long &last, bool &wasHigh,
                       unsigned long debounce = 200)
{
    bool high = (digitalRead(pin) == HIGH);

    if (high && !wasHigh && millis() - last >= debounce)
    {
        wasHigh = true;
        last = millis();
        return true;
    }
    if (!high)
    {
        wasHigh = false;
    }
    return false;
}

static bool intervalButtonPressed(int pin, unsigned long &last, unsigned long interval = 200)
{
    if (digitalRead(pin) == HIGH && millis() - last >= interval)
    {
        last = millis();
        return true;
    }
    return false;
}

bool rightPressed(unsigned long interval)
{
    static unsigned long last = 0;
    static bool wasHigh = false;
    return interval == 0 ? risingEdge(btnRght, last, wasHigh) : intervalButtonPressed(btnRght, last, interval);
}

bool leftPressed(unsigned long interval)
{
    static unsigned long last = 0;
    static bool wasHigh = false;
    return interval == 0 ? risingEdge(btnLft, last, wasHigh) : intervalButtonPressed(btnLft, last, interval);
}

bool enterPressed(unsigned long interval)
{
    // FIX: original code set last = millis() then immediately tested
    // millis() - last >= 100, which is always false, so lastState
    // never reset to LOW and the button could only fire once.
    static unsigned long last = 0;
    static bool wasHigh = false;
    return interval == 0 ? risingEdge(btnEtr, last, wasHigh) : intervalButtonPressed(btnEtr, last, interval);
}

bool removePressed(unsigned long interval)
{
    static unsigned long last = 0;
    static bool wasHigh = false;
    return interval == 0 ? risingEdge(btnRmv, last, wasHigh) : intervalButtonPressed(btnRmv, last, interval);
}