#include <Arduino.h>
#include "data.h"
#include "time-date.h"

ScheduleRAM todaySchedules[SCHEDULE_MAX_RAM];
byte todayScheduleCount = 0;

void getSchedules()
{
    todayScheduleCount = 0;

    for (int i = 0; i < SCHEDULE_MAX_EEPROM; i++)
    {
        int base = SCHEDULE_BASE + SCHEDULE_SLOT * i;
        byte flags = readEEPROM(base + 5);

        if (flags == FLAG_END_OF_LIST)
            break; // no more data
        if (flags == FLAG_INACTIVE)
            continue; // deleted / free slot

        byte rawDay = readEEPROM(base + 2);
        byte rawMonth = readEEPROM(base + 3);

        // Decode interval type
        bool isOnce = false;
        bool isDaily = false;
        bool isWeekly = false;
        bool isYearly = false;
        byte compareDay = rawDay;
        byte compareMonth = rawMonth;

        if (rawDay == 0)
        {
            isDaily = true;
        }
        else if (rawDay >= 51 && rawDay <= 57)
        {
            isWeekly = true;
            compareDay = rawDay - 50; // 1=Sun … 7=Sat
        }
        else if (rawDay >= 101 && rawDay <= 131)
        {
            isYearly = true;
            compareDay = rawDay - 100;
        }
        else
        {
            isOnce = true;
        }

        // ── Check if this schedule applies today ──────────────────────────
        bool matchToday = false;
        bool matchTomorrow = false;

        if (isDaily)
        {
            matchToday = true;
        }
        else if (isWeekly)
        {
            if (compareDay == nowTime.dayOfTheWeek() + 1)
                matchToday = true;
            if (compareDay == tomorrowTime.dayOfTheWeek() + 1)
                matchTomorrow = true;
        }
        else if (isYearly)
        {
            if (compareDay == nowTime.day() && compareMonth == nowTime.month())
                matchToday = true;
            if (compareDay == tomorrowTime.day() && compareMonth == tomorrowTime.month())
                matchTomorrow = true;
        }
        else
        { // once
            if (compareDay == nowTime.day() && compareMonth == nowTime.month())
                matchToday = true;
            if (compareDay == tomorrowTime.day() && compareMonth == tomorrowTime.month())
                matchTomorrow = true;
        }

        if (matchToday && todayScheduleCount < SCHEDULE_MAX_RAM)
        {
            todaySchedules[todayScheduleCount].hour = readEEPROM(base + 0);
            todaySchedules[todayScheduleCount].minute = readEEPROM(base + 1);
            todaySchedules[todayScheduleCount].day = 0;
            todaySchedules[todayScheduleCount].flags = flags;
            todaySchedules[todayScheduleCount].category = readEEPROM(base + 4);
            todaySchedules[todayScheduleCount].subject = readEEPROM(base + 6);
            todayScheduleCount++;

            // Once-only: mark as inactive after loading so it won't show again
            if (isOnce)
            {
                writeEEPROM(base + 5, FLAG_INACTIVE);
            }
        }

        // ── ONE_DAY_BEFORE reminder for tomorrow ──────────────────────────
        if (matchTomorrow && flags == ONE_DAY_BEFORE && todayScheduleCount < SCHEDULE_MAX_RAM)
        {
            todaySchedules[todayScheduleCount].hour = readEEPROM(base + 0);
            todaySchedules[todayScheduleCount].minute = readEEPROM(base + 1);
            todaySchedules[todayScheduleCount].day = 1;
            todaySchedules[todayScheduleCount].flags = flags;
            todaySchedules[todayScheduleCount].category = readEEPROM(base + 4);
            todaySchedules[todayScheduleCount].subject = readEEPROM(base + 6);
            todayScheduleCount++;
        }
    }
}