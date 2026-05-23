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

bool rightPressed()
{
    static unsigned long last = 0;
    static bool wasHigh = false;
    return risingEdge(btnRght, last, wasHigh);
}

bool leftPressed()
{
    static unsigned long last = 0;
    static bool wasHigh = false;
    return risingEdge(btnLft, last, wasHigh);
}

bool enterPressed()
{
    // FIX: original code set last = millis() then immediately tested
    // millis() - last >= 100, which is always false, so lastState
    // never reset to LOW and the button could only fire once.
    static unsigned long last = 0;
    static bool wasHigh = false;
    return risingEdge(btnEtr, last, wasHigh);
}

bool removePressed()
{
    static unsigned long last = 0;
    static bool wasHigh = false;
    return risingEdge(btnRmv, last, wasHigh);
}