#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include "lcd2.h"
#include "constant.h"
#include "time-date.h"
#include "data.h"
#include "reminder.h"

// ─── Global definitions (extern-declared in their respective headers) ─────────
LiquidCrystal_I2C lcd2(0x26, 16, 2);

DateTime nowTime;
DateTime tomorrowTime;

// ─── Private state ────────────────────────────────────────────────────────────
static int8_t lastSecond = -1;
static int8_t lastDay = -1; // -1 = uninitialised

static const char dayNames[7][4] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// ─────────────────────────────────────────────────────────────────────────────
// clockDisplay()
// Reads the RTC and updates lcd2. Must be called every loop iteration so that
// nowTime and tomorrowTime are always current for the rest of the system.
// ─────────────────────────────────────────────────────────────────────────────
void clockDisplay()
{
    nowTime = rtc.now();
    tomorrowTime = nowTime + TimeSpan(1, 0, 0, 0);

    // ── Midnight hook ─────────────────────────────────────────────────────────
    if (nowTime.hour() == 0 && nowTime.minute() == 0 && nowTime.second() == 0)
    {
        getSchedules();
        cleanupExpiredSchedules();
        resetReminderFlags(); // allow all reminders to fire again today
    }

    int currentDayIdx = nowTime.dayOfTheWeek();

    // ── Time: update every second ─────────────────────────────────────────────
    if (nowTime.second() != lastSecond)
    {
        lastSecond = nowTime.second();

        char buf[9]; // "HH:MM:SS\0"
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                 nowTime.hour(), nowTime.minute(), nowTime.second());
        lcd2.setCursor(0, 0);
        lcd2.print(buf);
    }

    // ── Day / date: update only when day changes ──────────────────────────────
    if (currentDayIdx != lastDay)
    {
        lastDay = currentDayIdx;

        lcd2.setCursor(13, 0);
        lcd2.print(dayNames[currentDayIdx]);

        char dateBuf[11]; // "DD-MM-YYYY\0"
        snprintf(dateBuf, sizeof(dateBuf), "%02d-%02d-%04d",
                 nowTime.day(), nowTime.month(), nowTime.year());
        lcd2.setCursor((16 - 10) / 2, 1);
        lcd2.print(dateBuf);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// lcd2Main()
// BUG FIX: this wrapper was deleted from the file during the reminder system
// integration, leaving lcd2Main() undefined.  On AVR, calling an undefined
// function jumps through an uninitialised vector — typically address 0, which
// is the RESET vector.  That caused setup() to re-run every iteration,
// re-initialising both LCDs and producing the "blank / reset on startup" symptom.
// ─────────────────────────────────────────────────────────────────────────────
void lcd2Main()
{
    Serial.println("lcd2: oke");
    clockDisplay();
}