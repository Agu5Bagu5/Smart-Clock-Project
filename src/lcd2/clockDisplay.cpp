#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include "lcd2.h"
#include "constant.h"
#include "time-date.h"

// These are the authoritative definitions (extern'd in time-date.h)
DateTime nowTime;
DateTime tomorrowTime;

static int8_t lastSecond = -1;
static int8_t lastDay = -1; // -1 = uninitialised

static const char dayNames[7][4] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void clockDisplay()
{
    nowTime = rtc.now();
    tomorrowTime = nowTime + TimeSpan(1, 0, 0, 0);

    int currentDayIdx = nowTime.dayOfTheWeek();

    // ── Time: update every second ─────────────────────────────────────────
    if (nowTime.second() != lastSecond)
    {
        lastSecond = nowTime.second();

        char buf[9]; // "HH:MM:SS"
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                 nowTime.hour(), nowTime.minute(), nowTime.second());
        lcd2.setCursor(0, 0);
        lcd2.print(buf);
    }

    // ── Day name: update only when day changes ────────────────────────────
    if (currentDayIdx != lastDay)
    {
        lastDay = currentDayIdx;

        // Day name, right-aligned on row 0
        lcd2.setCursor(13, 0);
        lcd2.print(dayNames[currentDayIdx]);

        // Full date on row 1, centred
        char dateBuf[12]; // "DD-MM-YYYY"
        snprintf(dateBuf, sizeof(dateBuf), "%02d-%02d-%04d",
                 nowTime.day(), nowTime.month(), nowTime.year());
        lcd2.setCursor((16 - 10) / 2, 1);
        lcd2.print(dateBuf);
    }
}