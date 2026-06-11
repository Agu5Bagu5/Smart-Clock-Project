#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "symbols.h"
#include "reminder.h"

LiquidCrystal_I2C lcd1(0x27, 16, 2);

State currentState = ADD_SCHEDULE;

void lcd1Main()
{
    // // Reminder display has priority over all normal states
    // if (getCurrentReminderLevel() >= 0)
    // {
    //     drawReminderScreen();
    //     return;
    // }

    // switch (currentState)
    // {
    // case HOMEPAGE:
    //     homepage();
    //     break;
    // case MENU:
    //     menu();
    //     break;
    // case ADD_SCHEDULE:
    //     addScheduleInterface();
    //     break;
    // case VIEW_SCHEDULE:
    //     viewSchedules();
    //     break;
    // case SUBJECTS:
    //     subjects();
    //     break;
    // default:
    //     break;
    // }
}