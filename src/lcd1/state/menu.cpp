#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "buttonPressLogic.h"

void menu()
{
    static byte cursor = 0;
    static byte lastCursor = 1;

    // Logika untuk menampilkan homepage
    if (cursor != lastCursor)
    {
        lcd1.setCursor(0, 0);
        lcd1.print(cursor == 0 ? "> Add Schedule" : "  Add Schedule");
        lcd1.setCursor(0, 1);
        lcd1.print(cursor == 0 ? "> View Schedule" : "  View Schedule");
    }
}