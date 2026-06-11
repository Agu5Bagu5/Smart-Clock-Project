#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "buttonPressLogic.h"

void menu()
{
    static byte cursor = 0;
    static byte lastCursor = 255;

    const char *menuItems[] =
        {
            "Add Schedule",
            "View Schedule",
            "Subjects"};

    const byte MENU_COUNT = sizeof(menuItems) / sizeof(menuItems[0]);

    // ===== NAVIGATION =====
    if (rightPressed())
    {
        cursor = (cursor + 1) % MENU_COUNT;
    }

    if (leftPressed())
    {
        cursor = (cursor == 0)
                     ? MENU_COUNT - 1
                     : cursor - 1;
    }

    // ===== ENTER =====
    if (enterPressed())
    {
        switch (cursor)
        {
        case 0:
            currentState = ADD_SCHEDULE;
            break;

        case 1:
            currentState = VIEW_SCHEDULE;
            break;

        case 2:
            currentState = SUBJECTS;
            break;
        }

        lastCursor = 255;
        return;
    }

    // ===== RENDER =====
    if (cursor != lastCursor)
    {
        byte nextItem = (cursor + 1) % MENU_COUNT;

        lcd1.clear();

        lcd1.setCursor(0, 0);
        lcd1.print(">");
        lcd1.print(menuItems[cursor]);

        lcd1.setCursor(0, 1);
        lcd1.print(" ");
        lcd1.print(menuItems[nextItem]);

        lastCursor = cursor;
    }
}