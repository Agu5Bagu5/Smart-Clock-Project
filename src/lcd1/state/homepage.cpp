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
    static HomepageState state = HOMEPAGE_UPCOMING;
    static unsigned long lastStateChange = 0;

    // FIX: do not use static initialisers for values derived from runtime data —
    // they only run once at program start when todayScheduleCount is still 0.
    // Compute them fresh each call instead.
    int countSummary = todayScheduleCount;
    int countLeft = todayScheduleCount - indexUpcoming;

    if (nowTime.hour() == 0 && nowTime.minute() == 0 && nowTime.second() == 0)
    {
        // New day: reset indices
        indexUpcoming = 0;
        indexNext = 1;
        countSummary = todayScheduleCount;
        countLeft = todayScheduleCount;
        state = HOMEPAGE_UPCOMING;
    }

    // FIX: isThereNext guards ALL accesses to indexNext / nextEvent below.
    bool isThereNext = (indexNext < todayScheduleCount);

    // FIX: upcomingTimeLeft and nextTimeLeft must both be signed so that
    // past-due events yield a negative value rather than wrapping to ~65535.
    signed int upcomingTimeLeft =
        (signed int)(todaySchedules[indexUpcoming].hour * 60 + todaySchedules[indexUpcoming].minute) -
        (signed int)(nowTime.hour() * 60 + nowTime.minute());

    // FIX: only compute nextTimeLeft when indexNext is in range.
    signed int nextTimeLeft = 0;
    if (isThereNext)
    {
        nextTimeLeft =
            (signed int)(todaySchedules[indexNext].hour * 60 + todaySchedules[indexNext].minute) -
            (signed int)(nowTime.hour() * 60 + nowTime.minute());
    }

    // Advance past expired events
    if (upcomingTimeLeft < 0)
    {
        if (indexUpcoming < todayScheduleCount - 1)
        {
            indexUpcoming++;
            countLeft--;
            isThereNext = (indexNext < todayScheduleCount && indexNext > indexUpcoming);

            if (isThereNext && indexNext < indexUpcoming)
                indexNext = indexUpcoming + 1;

            isThereNext = (indexNext < todayScheduleCount);

            upcomingTimeLeft =
                (signed int)(todaySchedules[indexUpcoming].hour * 60 + todaySchedules[indexUpcoming].minute) -
                (signed int)(nowTime.hour() * 60 + nowTime.minute());

            if (isThereNext)
            {
                nextTimeLeft =
                    (signed int)(todaySchedules[indexNext].hour * 60 + todaySchedules[indexNext].minute) -
                    (signed int)(nowTime.hour() * 60 + nowTime.minute());
            }
        }
        else
        {
            // No more events left; show summary screen.
            state = HOMEPAGE_SUMMARY;
        }
    }

    ScheduleRAM upcomingEvent = todaySchedules[indexUpcoming];

    // FIX: only read nextEvent when index is valid.
    ScheduleRAM nextEvent;
    if (isThereNext)
        nextEvent = todaySchedules[indexNext];

    unsigned long interval =
        (state == HOMEPAGE_UPCOMING) ? DELAY_UPCOMING : (state == HOMEPAGE_NEXT) ? DELAY_NEXT
                                                                                 : DELAY_SUMMARY;

    // Subject name lookup
    int subjectUpcomingIndex = 0;
    for (byte i = 0; i < subjectCount; i++)
    {
        if (subjectRAMs[i].subjectId == upcomingEvent.subject)
        {
            subjectUpcomingIndex = subjectRAMs[i].index;
            break;
        }
    }

    int subjectNextIndex = 0;
    if (isThereNext)
    {
        for (byte i = 0; i < subjectCount; i++)
        {
            if (subjectRAMs[i].subjectId == nextEvent.subject)
            {
                subjectNextIndex = subjectRAMs[i].index;
                break;
            }
        }
    }

    // FIX: replace Arduino String() with char arrays to avoid heap fragmentation.
    char bufUTop[17], bufUBottom[17];
    snprintf(bufUTop, sizeof(bufUTop), " %.14s", getSubjectName(subjectUpcomingIndex));
    snprintf(bufUBottom, sizeof(bufUBottom), "%d:%02d in %d min",
             upcomingEvent.hour, upcomingEvent.minute, upcomingTimeLeft);

    char bufTop[17], bufBottom[17];
    if (isThereNext)
    {
        snprintf(bufTop, sizeof(bufTop), " %.14s", getSubjectName(subjectNextIndex));
        snprintf(bufBottom, sizeof(bufBottom), "%d:%02d in %d min",
                 nextEvent.hour, nextEvent.minute, nextTimeLeft);
    }

    char bufSTop[17], bufSBottom[17];
    snprintf(bufSTop, sizeof(bufSTop), "Today: %d event%s",
             countSummary, countSummary != 1 ? "s" : "");
    snprintf(bufSBottom, sizeof(bufSBottom), "%d left", countLeft);

    if (millis() - lastStateChange >= interval)
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
            if (isThereNext)
            {
                lcd1.clear();
                lcd1.setCursor(0, 0);
                lcd1.write(nextEvent.category);
                lcd1.print(bufTop);
                lcd1.setCursor(0, 1);
                lcd1.print(bufBottom);
            }
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
        }
    }
}