#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "symbols.h"

LiquidCrystal_I2C lcd1(0x27, 16, 2);

State currentState = ADD_SCHEDULE;
State lastState = HOMEPAGE;

void lcd1Main()
{
    // Jalankan logika berdasarkan state saat ini
    switch (currentState)
    {
    case MENU:
        menu();
        lastState = HOMEPAGE;
        break;
    case ADD_SCHEDULE:
        addScheduleInterface();
        break;

    default:
        break;
    }
}