#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "data.h"
#include "time-date.h"
#include "RTClib.h"
#include "buttonPressLogic.h"

extern RTC_DS3231 rtc;

enum ViewScheduleMode
{
    DATE_SELECT,
    SCHEDULE_VIEW,
    DELETE_CONFIRM
};

// FIX: returns int so callers can detect -1 (not found).
int getSubjectIndex(byte subjectId)
{
    for (byte i = 0; i < subjectCount; i++)
    {
        if (subjectRAMs[i].subjectId == subjectId)
            return subjectRAMs[i].index;
    }
    return -1;
}

void reloadVisibleSchedules(
    const DateTime &selectedDate,
    byte visibleSchedules[],
    byte &visibleCount)
{
    getSchedules(selectedDate);

    visibleCount = 0;

    for (byte i = 0; i < todayScheduleCount; i++)
    {
        if (todaySchedules[i].day == 0)
            visibleSchedules[visibleCount++] = i;
    }
}

void viewSchedules()
{
    static bool initialized = false;

    static ViewScheduleMode mode;

    static DateTime selectedDate;

    static byte visibleSchedules[SCHEDULE_MAX_RAM];
    static byte visibleCount = 0;

    static byte currentSchedule = 0;
    static bool deleteYes = false;

    static bool forceRender = true;

    const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    // =====================================================
    // FIRST ENTRY
    // =====================================================

    if (!initialized)
    {
        deleteYes = false;
        selectedDate = rtc.now();

        mode = DATE_SELECT;

        visibleCount = 0;
        currentSchedule = 0;

        lcd1.clear();

        forceRender = true;
        initialized = true;
    }

    // =====================================================
    // DATE SELECT MODE
    // =====================================================

    if (mode == DATE_SELECT)
    {
        if (leftPressed())
        {
            selectedDate = selectedDate - TimeSpan(1, 0, 0, 0);
            forceRender = true;
        }

        if (rightPressed())
        {
            selectedDate = selectedDate + TimeSpan(1, 0, 0, 0);
            forceRender = true;
        }

        if (enterPressed())
        {
            getSchedules(selectedDate);

            visibleCount = 0;
            for (byte i = 0; i < todayScheduleCount; i++)
            {
                if (todaySchedules[i].day == 0)
                    visibleSchedules[visibleCount++] = i;
            }

            currentSchedule = 0;
            mode = SCHEDULE_VIEW;

            lcd1.clear();
            forceRender = true;
            return;
        }

        if (removePressed())
        {
            initialized = false;
            currentState = MENU;
            lcd1.clear();
            return;
        }

        if (forceRender)
        {
            lcd1.clear();

            lcd1.setCursor(0, 0);
            lcd1.print("Select Date");

            lcd1.setCursor(0, 1);
            char line[17];
            sprintf(line, "%02d/%02d %s",
                    selectedDate.day(),
                    selectedDate.month(),
                    days[selectedDate.dayOfTheWeek()]);
            lcd1.print(line);

            forceRender = false;
        }

        return;
    }

    // =====================================================
    // DELETE CONFIRM MODE
    // =====================================================

    if (mode == DELETE_CONFIRM)
    {
        if (leftPressed() || rightPressed())
        {
            deleteYes = !deleteYes;
            forceRender = true;
        }

        if (removePressed())
        {
            mode = SCHEDULE_VIEW;
            lcd1.clear();
            forceRender = true;
            return;
        }

        if (enterPressed())
        {
            if (!deleteYes)
            {
                mode = SCHEDULE_VIEW;
                lcd1.clear();
                forceRender = true;
                return;
            }

            ScheduleRAM &schedule = todaySchedules[visibleSchedules[currentSchedule]];

            int base = SCHEDULE_BASE + schedule.eepromSlot * SCHEDULE_SLOT;
            writeEEPROM(base + 5, FLAG_INACTIVE);

            reloadVisibleSchedules(selectedDate, visibleSchedules, visibleCount);

            if (visibleCount == 0)
                currentSchedule = 0;
            else if (currentSchedule >= visibleCount)
                currentSchedule = visibleCount - 1;

            mode = SCHEDULE_VIEW;
            lcd1.clear();
            forceRender = true;
            return;
        }

        if (!forceRender)
            return;

        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("Delete Sched?");
        lcd1.setCursor(0, 1);
        lcd1.print(deleteYes ? " No    >Yes" : ">No     Yes");

        forceRender = false;
        return;
    }

    // =====================================================
    // SCHEDULE VIEW MODE
    // =====================================================

    if (leftPressed())
    {
        if (visibleCount > 0)
        {
            if (currentSchedule == 0)
                currentSchedule = visibleCount - 1;
            else
                currentSchedule--;
            forceRender = true;
        }
    }

    if (rightPressed())
    {
        if (visibleCount > 0)
        {
            currentSchedule++;
            if (currentSchedule >= visibleCount)
                currentSchedule = 0;
            forceRender = true;
        }
    }

    if (enterPressed() && visibleCount > 0)
    {
        deleteYes = false;
        mode = DELETE_CONFIRM;
        lcd1.clear();
        forceRender = true;
        return;
    }

    if (removePressed())
    {
        mode = DATE_SELECT;
        lcd1.clear();
        forceRender = true;
        return;
    }

    if (!forceRender)
        return;

    lcd1.clear();

    // =====================================================
    // NO SCHEDULES
    // =====================================================

    if (visibleCount == 0)
    {
        char header[17];
        sprintf(header, "%02d/%02d %s",
                selectedDate.day(),
                selectedDate.month(),
                days[selectedDate.dayOfTheWeek()]);

        lcd1.setCursor(0, 0);
        lcd1.print(header);
        lcd1.setCursor(0, 1);
        lcd1.print("No schedules");

        forceRender = false;
        return;
    }

    // =====================================================
    // DISPLAY SCHEDULE
    // =====================================================

    ScheduleRAM &schedule = todaySchedules[visibleSchedules[currentSchedule]];

    char header[17];
    sprintf(header, "%02d/%02d %02d:%02d %d/%d",
            selectedDate.day(),
            selectedDate.month(),
            schedule.hour,
            schedule.minute,
            currentSchedule + 1,
            visibleCount);

    lcd1.setCursor(0, 0);
    lcd1.print(header);

    lcd1.setCursor(0, 1);
    lcd1.write(schedule.category);
    lcd1.print(" ");

    // FIX: guard against getSubjectIndex returning -1 (unknown subject).
    int subjectIdx = getSubjectIndex(schedule.subject);
    if (subjectIdx >= 0)
        lcd1.print(getSubjectName((byte)subjectIdx));
    else
        lcd1.print("?");

    forceRender = false;
}