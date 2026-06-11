#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "data.h"
#include "blinkLogic.h"
#include "time-date.h"
#include "RTClib.h"

enum HomepageState
{
    HOMEPAGE_UPCOMING = 0,
    HOMEPAGE_NEXT = 1,
    HOMEPAGE_SUMMARY = 2,
    HOMEPAGE_NO_EVENT = 3
};

enum StateDelay : unsigned long
{
    DELAY_UPCOMING = 8000,
    DELAY_NEXT = 3000,
    DELAY_SUMMARY = 2000,
};

void homepage()
{
    if (todayScheduleCount == 0)
    {
        lcd1.setCursor(0, 0);
        lcd1.print("No events");

        lcd1.setCursor(0, 1);
        lcd1.print("for today");

        return;
    }

    static int indexUpcoming = 0;
    static int indexNext = 1;
    static int countSummary = todayScheduleCount;
    static int countLeft = todayScheduleCount - indexUpcoming;
    bool isThereNext = true;

    static HomepageState state = HOMEPAGE_UPCOMING;

    if (nowTime.hour() == 0 && nowTime.minute() == 0 && nowTime.second() == 0)
    {
        // New day, refresh schedules
        indexUpcoming = 0;
        indexNext = 1;
        countSummary = todayScheduleCount;
        countLeft = todayScheduleCount - indexUpcoming;
    }

    signed int upcomingTimeLeft = (todaySchedules[indexUpcoming].hour * 60 + todaySchedules[indexUpcoming].minute) - (nowTime.hour() * 60 + nowTime.minute());
    unsigned int nextTimeLeft = (todaySchedules[indexNext].hour * 60 + todaySchedules[indexNext].minute) - (nowTime.hour() * 60 + nowTime.minute());

    if (upcomingTimeLeft < 0 && indexUpcoming < todayScheduleCount - 1 && indexNext < todayScheduleCount - 1)
    {
        indexUpcoming++;
        indexNext++;
        countLeft--;
        upcomingTimeLeft = (todaySchedules[indexUpcoming].hour * 60 + todaySchedules[indexUpcoming].minute) - (nowTime.hour() * 60 + nowTime.minute());
        nextTimeLeft = (todaySchedules[indexNext].hour * 60 + todaySchedules[indexNext].minute) - (nowTime.hour() * 60 + nowTime.minute());
    }
    else if (upcomingTimeLeft < 0 && indexUpcoming < todayScheduleCount - 1 && indexNext == todayScheduleCount - 1)
    {
        indexUpcoming++;
        countLeft--;
        isThereNext = false;
        upcomingTimeLeft = (todaySchedules[indexUpcoming].hour * 60 + todaySchedules[indexUpcoming].minute) - (nowTime.hour() * 60 + nowTime.minute());
    }
    else if (upcomingTimeLeft < 0)
    {
        state = HOMEPAGE_NO_EVENT;
    }

    ScheduleRAM upcomingEvent = todaySchedules[indexUpcoming];
    ScheduleRAM nextEvent = todaySchedules[indexNext];

    unsigned long interval = state == HOMEPAGE_UPCOMING ? DELAY_UPCOMING : (state == HOMEPAGE_NEXT ? DELAY_NEXT : DELAY_SUMMARY);

    static int subjectNextIndex = 0;
    static int subjectUpcomingIndex = 0;

    for (byte i = 0; i < subjectCount; i++)
    {
        if (
            subjectRAMs[i].subjectId == nextEvent.subject)
        {
            subjectNextIndex = subjectRAMs[i].index;
            break;
        }
    }

    for (byte i = 0; i < subjectCount; i++)
    {
        if (
            subjectRAMs[i].subjectId == upcomingEvent.subject)
        {
            subjectUpcomingIndex = subjectRAMs[i].index;
            break;
        }
    }

    String bufUTop = " " + String(getSubjectName(subjectUpcomingIndex));
    String bufUBottom = String(upcomingEvent.hour) + ":" + (upcomingEvent.minute < 10 ? "0" : "") + String(upcomingEvent.minute) + " in " + String(upcomingTimeLeft) + " min";

    String bufTop = " " + String(getSubjectName(subjectNextIndex));
    String bufBottom = String(nextEvent.hour) + ":" + (nextEvent.minute < 10 ? "0" : "") + String(nextEvent.minute) + " in " + String(nextTimeLeft) + " min";

    String bufSTop = "Today: " + String(countSummary) + " event" + (countSummary > 1 ? "s" : "");
    String bufSBottom = String(countLeft) + " left";

    static unsigned long lastStateChange = 0;
    if (millis() - lastStateChange >= interval || state == HOMEPAGE_NO_EVENT)
    {
        switch (state)
        {
        case HOMEPAGE_UPCOMING:
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.write(upcomingEvent.category);
            lcd1.print(bufUTop);
            lcd1.setCursor(0, 1);
            lcd1.print(bufUBottom);

            lastStateChange = millis();
            state = isThereNext ? HOMEPAGE_NEXT : HOMEPAGE_SUMMARY;
            interval = isThereNext ? DELAY_NEXT : DELAY_SUMMARY;
            break;
        case HOMEPAGE_NEXT:
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.write(nextEvent.category);
            lcd1.print(bufTop);
            lcd1.setCursor(0, 1);
            lcd1.print(bufBottom);

            lastStateChange = millis();
            state = HOMEPAGE_SUMMARY;
            interval = DELAY_SUMMARY;
            break;
        case HOMEPAGE_SUMMARY:
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print(bufSTop);
            lcd1.setCursor(0, 1);
            lcd1.print(bufSBottom);

            lastStateChange = millis();
            state = HOMEPAGE_UPCOMING;
            interval = DELAY_UPCOMING;
            break;
        case HOMEPAGE_NO_EVENT:
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("No events");
            lcd1.setCursor(0, 1);
            lcd1.print("for today");
            break;
        }
    }
}
