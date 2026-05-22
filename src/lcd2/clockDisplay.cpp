#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include "lcd2.h"
#include "constant.h"
#include "time-date.h"

DateTime nowTime;
DateTime tomorrowTime;

unsigned long lastTimeDisplay = 0;
unsigned long timeDisplayDelay = 1000;

char namaHari[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
// Use nullptr for pointers to avoid warnings
char *lastDay = nullptr;

void clockDisplay()
{
    unsigned long currentTime = millis();
    nowTime = rtc.now(); // Update waktu saat ini dari RTC
    tomorrowTime = nowTime + TimeSpan(1, 0, 0, 0);
    int currentDayIdx = nowTime.dayOfTheWeek();

    // 1. Update the time every second
    if (currentTime - lastTimeDisplay > timeDisplayDelay)
    {
        lcd2.setCursor(0, 0);
        if (nowTime.hour() < 10)
            lcd2.print('0');
        lcd2.print(nowTime.hour());
        lcd2.print(':');
        if (nowTime.minute() < 10)
            lcd2.print('0');
        lcd2.print(nowTime.minute());
        lcd2.print(':');
        if (nowTime.second() < 10)
            lcd2.print('0');
        lcd2.print(nowTime.second());
        lastTimeDisplay = currentTime;
    }

    // 2. Update Day and Date only if the day has changed (or on startup)
    if (lastDay == nullptr || namaHari[currentDayIdx] != lastDay)
    {
        // Update Day Name
        lcd2.setCursor(16 - 3, 0);
        lcd2.print(namaHari[currentDayIdx]);

        // Update Full Date
        lcd2.setCursor(2, 1);
        if (nowTime.day() < 10)
            lcd2.print('0');
        lcd2.print(nowTime.day());
        lcd2.print('-');

        // Fixed month logic
        if (nowTime.month() < 10)
            lcd2.print('0');
        lcd2.print(nowTime.month());

        lcd2.print('-');
        lcd2.print(nowTime.year());

        // Save the current day so we don't refresh the date again until tomorrow
        lastDay = namaHari[currentDayIdx];
    }
}