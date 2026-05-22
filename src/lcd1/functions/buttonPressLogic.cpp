#include <Arduino.h>
#include <constant.h>
#include <buttonPressLogic.h>

bool rightPressed()
{
    static unsigned long last = 0;

    if (digitalRead(btnRght) == HIGH)
    {
        if (millis() - last >= 200)
        {
            last = millis();
            return true;
        }
    }

    return false;
}

bool leftPressed()
{
    static unsigned long last = 0;

    if (digitalRead(btnLft) == HIGH)
    {
        if (millis() - last >= 200)
        {
            last = millis();
            return true;
        }
    }

    return false;
}

bool enterPressed()
{
    static unsigned long last = 0;
    static bool lastState = LOW;

    if (digitalRead(btnEtr) == HIGH && lastState == LOW)
    {
        lastState = HIGH;

        if (millis() - last >= 200)
        {
            last = millis();
            return true;
        }
    }
    else
    {
        if (lastState == HIGH)
        {
            last = millis();
            if (millis() - last >= 100)
            {
                lastState = LOW;
            }
        }
    }

    return false;
}

bool removePressed()
{
    static unsigned long last = 0;

    if (digitalRead(btnRmv) == HIGH)
    {
        if (millis() - last >= 200)
        {
            last = millis();
            return true;
        }
    }

    return false;
}