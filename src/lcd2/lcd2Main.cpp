#include "lcd2.h"
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "constant.h"

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd2(0x26, 16, 2);

void lcd2Main()
{
    clockDisplay();
}