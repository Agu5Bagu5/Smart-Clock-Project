#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "buttonPressLogic.h"

void menu()
{
    static byte cursor = 0;
    static byte lastCursor = 0xFF; // force first render

    if (rightPressed() || leftPressed())
    {
        cursor = cursor ? 0 : 1;
    }

    if (enterPressed())
    {
        if (cursor == 0)
        {
            currentState = ADD_SCHEDULE;
        }
        // cursor == 1: VIEW_SCHEDULE (add that state when ready)
        lastCursor = 0xFF; // force re-render on next entry
        return;
    }

    if (cursor != lastCursor)
    {
        lcd1.setCursor(0, 0);
        lcd1.print(cursor == 0 ? ">Add Schedule   "
                               : " Add Schedule   ");
        lcd1.setCursor(0, 1);
        lcd1.print(cursor == 1 ? ">View Schedules "
                               : " View Schedules ");
        lastCursor = cursor;
    }
}