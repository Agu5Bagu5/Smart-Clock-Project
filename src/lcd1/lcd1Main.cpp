#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "symbols.h"

LiquidCrystal_I2C lcd1(0x27, 16, 2);

State currentState = ADD_SCHEDULE;

void lcd1Main()
{
    switch (currentState)
    {
    case MENU:
        menu();
        break;
    case ADD_SCHEDULE:
        addScheduleInterface();
        break;
    case HOMEPAGE:
        homepage();
        break;
    default:
        break;
    }
}