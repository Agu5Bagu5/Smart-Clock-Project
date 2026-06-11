#include <Arduino.h>
#include "data.h"
#include "time-date.h"

ScheduleRAM todaySchedules[SCHEDULE_MAX_RAM];
byte todayScheduleCount = 0;

// ──────────────────────────────────────────────────────────────
//  Sort schedules:
//  1. Today schedules first (day=0)
//  2. Tomorrow reminders second (day=1)
//  3. Earlier time first
// ──────────────────────────────────────────────────────────────
void sortSchedules()
{
    for (byte i = 0; i < todayScheduleCount - 1; i++)
    {
        for (byte j = i + 1; j < todayScheduleCount; j++)
        {
            bool swapNeeded = false;

            if (todaySchedules[i].day > todaySchedules[j].day)
            {
                swapNeeded = true;
            }
            else if (todaySchedules[i].day == todaySchedules[j].day)
            {
                uint16_t timeA = todaySchedules[i].hour * 60 + todaySchedules[i].minute;
                uint16_t timeB = todaySchedules[j].hour * 60 + todaySchedules[j].minute;
                if (timeA > timeB)
                    swapNeeded = true;
            }

            if (swapNeeded)
            {
                ScheduleRAM temp = todaySchedules[i];
                todaySchedules[i] = todaySchedules[j];
                todaySchedules[j] = temp;
            }
        }
    }
}

// ──────────────────────────────────────────────────────────────
//  Delete expired one-time schedules.
//  FIX: default argument must only appear in the declaration (data.h),
//  not in the definition — removed here.
// ──────────────────────────────────────────────────────────────
void cleanupExpiredSchedules(const DateTime &today)
{
    DateTime yesterday = today - TimeSpan(1, 0, 0, 0);

    for (int i = 0; i < SCHEDULE_MAX_EEPROM; i++)
    {
        int base = SCHEDULE_BASE + SCHEDULE_SLOT * i;

        byte flags = readEEPROM(base + 5);

        if (flags == FLAG_END_OF_LIST)
            break;

        if (flags == FLAG_INACTIVE)
            continue;

        byte rawDay = readEEPROM(base + 2);
        byte rawMonth = readEEPROM(base + 3);

        // Skip recurring schedules
        if (rawDay == 0)
            continue;
        if (rawDay >= 51 && rawDay <= 57)
            continue;
        if (rawDay >= 101 && rawDay <= 131)
            continue;

        // Delete one-time schedule from yesterday
        if (rawDay == yesterday.day() && rawMonth == yesterday.month())
        {
            writeEEPROM(base + 5, FLAG_INACTIVE);
        }
    }
}

// ──────────────────────────────────────────────────────────────
//  Load schedules for a given date.
//  FIX: default argument removed from definition — keep only in data.h.
// ──────────────────────────────────────────────────────────────
void getSchedules(const DateTime &referenceDate)
{
    todayScheduleCount = 0;

    DateTime nextDay = referenceDate + TimeSpan(1, 0, 0, 0);

    for (int i = 0; i < SCHEDULE_MAX_EEPROM; i++)
    {
        int base = SCHEDULE_BASE + SCHEDULE_SLOT * i;

        byte flags = readEEPROM(base + 5);

        if (flags == FLAG_END_OF_LIST)
            break;

        if (flags == FLAG_INACTIVE)
            continue;

        byte rawDay = readEEPROM(base + 2);
        byte rawMonth = readEEPROM(base + 3);

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
            compareDay = rawDay - 50;
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

        bool matchToday = false;
        bool matchTomorrow = false;

        if (isDaily)
        {
            matchToday = true;
        }
        else if (isWeekly)
        {
            if (compareDay == referenceDate.dayOfTheWeek() + 1)
                matchToday = true;
            if (compareDay == nextDay.dayOfTheWeek() + 1)
                matchTomorrow = true;
        }
        else if (isYearly)
        {
            if (compareDay == referenceDate.day() && compareMonth == referenceDate.month())
                matchToday = true;
            if (compareDay == nextDay.day() && compareMonth == nextDay.month())
                matchTomorrow = true;
        }
        else // once
        {
            if (compareDay == referenceDate.day() && compareMonth == referenceDate.month())
                matchToday = true;
            if (compareDay == nextDay.day() && compareMonth == nextDay.month())
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
            todaySchedules[todayScheduleCount].eepromSlot = i;
            todayScheduleCount++;
        }

        if (matchTomorrow && flags == ONE_DAY_BEFORE && todayScheduleCount < SCHEDULE_MAX_RAM)
        {
            todaySchedules[todayScheduleCount].hour = readEEPROM(base + 0);
            todaySchedules[todayScheduleCount].minute = readEEPROM(base + 1);
            todaySchedules[todayScheduleCount].day = 1;
            todaySchedules[todayScheduleCount].flags = flags;
            todaySchedules[todayScheduleCount].category = readEEPROM(base + 4);
            todaySchedules[todayScheduleCount].subject = readEEPROM(base + 6);
            todaySchedules[todayScheduleCount].eepromSlot = i;
            todayScheduleCount++;
        }
    }

    sortSchedules();
}