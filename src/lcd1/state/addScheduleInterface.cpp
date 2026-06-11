#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "symbols.h"
#include "data.h"
#include "buttonPressLogic.h"
#include "blinkLogic.h"
#include "time-date.h"

// ─── Shared schedule being built ─────────────────────────────────────────────
struct TempSchedule
{
    byte hour;
    byte minute;
    byte day;
    byte month;
    byte category;
    byte subject;
    byte flags;
    byte interval;
};
static TempSchedule newSchedule;

enum Step : byte
{
    SELECT_CATEGORY = 0,
    SELECT_SUBJECT,
    SELECT_INTERVAL,
    TIME_INPUT,
    SELECT_REMINDER,
    CONFIRMATION
};
static Step currentStep = SELECT_CATEGORY;
static Step lastStep = (Step)0xFF;
static bool stepJustChanged = false;

// FIX: track whether the flow has been initialised for this visit to ADD_SCHEDULE.
// Allows full reset when the user leaves and returns.
static bool flowInitialised = false;

static void printCentered(const char *s, byte row)
{
    char buf[17];
    memset(buf, ' ', 16);
    buf[16] = '\0';
    int len = strlen(s);
    int col = (16 - len) / 2;
    if (col < 0)
        col = 0;
    memcpy(buf + col, s, min(len, 16 - col));
    lcd1.setCursor(0, row);
    lcd1.print(buf);
}

static const char *stepTitle(Step s)
{
    switch (s)
    {
    case SELECT_CATEGORY:
        return "Select Category";
    case SELECT_SUBJECT:
        return "Select Subject";
    case SELECT_INTERVAL:
        return "Select Interval";
    case TIME_INPUT:
        return "Input Time/Date";
    case SELECT_REMINDER:
        return "Select Reminder";
    case CONFIRMATION:
        return "Confirm it!";
    default:
        return "";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 1: Category
// ─────────────────────────────────────────────────────────────────────────────
static void categorySelection()
{
    static byte activeCat = HOMEWORK;
    static byte lastActiveCat = 0xFF;

    if (lastActiveCat != activeCat)
    {
        static const char *catNames[4] = {"Homework", "Exam", "Personal", "Other"};
        printCentered(catNames[activeCat], 0);

        lcd1.setCursor(0, 1);
        lcd1.print("                ");
        for (byte i = 0; i < 4; i++)
        {
            lcd1.setCursor(4 + i * 3, 1);
            lcd1.print(activeCat == i ? ">" : " ");
            lcd1.write(i);
        }
        lastActiveCat = activeCat;
    }

    if (rightPressed())
        activeCat = (activeCat < OTHER) ? activeCat + 1 : HOMEWORK;
    else if (leftPressed())
        activeCat = (activeCat > HOMEWORK) ? activeCat - 1 : OTHER;
    else if (enterPressed())
    {
        newSchedule.category = activeCat;
        currentStep = SELECT_SUBJECT;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 2: Subject
// ─────────────────────────────────────────────────────────────────────────────
static void subjectSelection()
{
    static byte activeSubject = 0;
    static byte lastActiveSubject = 0xFF;
    static bool inAddMode = false;

    if (inAddMode)
    {
        const char *result = typing();
        if (result[0] != '\0')
        {
            byte idTaken[SUBJECT_MAX_RAM];
            memset(idTaken, 0, sizeof(idTaken));
            for (int i = 0; i < subjectCount; i++)
            {
                if (subjectRAMs[i].subjectId < SUBJECT_MAX_RAM)
                    idTaken[subjectRAMs[i].subjectId] = 1;
            }
            byte newId = 0;
            for (byte i = 0; i < SUBJECT_MAX_RAM; i++)
            {
                if (!idTaken[i])
                {
                    newId = i;
                    break;
                }
            }
            addSubject(newSchedule.category, newId, result);
            resetTyping();
            inAddMode = false;
            activeSubject = 0;
            lastActiveSubject = 0xFF;
        }
        return;
    }

    byte count = 0;
    for (byte i = 0; i < subjectCount; i++)
    {
        if (subjectRAMs[i].category == newSchedule.category)
            count++;
    }
    byte maxSlot = count;

    if (rightPressed() && activeSubject < maxSlot)
        activeSubject++;
    else if (leftPressed() && activeSubject > 0)
        activeSubject--;
    else if (enterPressed())
    {
        if (activeSubject == 0)
        {
            inAddMode = true;
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("                ");
            lcd1.setCursor(0, 1);
            lcd1.print("                ");
            return;
        }
        else
        {
            byte n = 0;
            for (byte i = 0; i < subjectCount; i++)
            {
                if (subjectRAMs[i].category == newSchedule.category)
                {
                    n++;
                    if (n == activeSubject)
                    {
                        newSchedule.subject = subjectRAMs[i].subjectId;
                        break;
                    }
                }
            }
            currentStep = SELECT_INTERVAL;
        }
    }

    if (lastActiveSubject != activeSubject)
    {
        lcd1.clear();

        char curName[16];
        if (activeSubject == 0)
        {
            strcpy(curName, "Add Subject");
        }
        else
        {
            byte n = 0;
            curName[0] = '\0';
            for (byte i = 0; i < subjectCount; i++)
            {
                if (subjectRAMs[i].category == newSchedule.category)
                {
                    n++;
                    if (n == activeSubject)
                    {
                        strncpy(curName, getSubjectName(subjectRAMs[i].index), 15);
                        curName[15] = '\0';
                        break;
                    }
                }
            }
        }
        printCentered(curName, 0);

        lcd1.setCursor(0, 1);
        if (activeSubject == 0)
        {
            lcd1.print("   ");
        }
        else if (activeSubject == 1)
        {
            lcd1.print("Add");
        }
        else
        {
            byte n = 0;
            for (byte i = 0; i < subjectCount; i++)
            {
                if (subjectRAMs[i].category == newSchedule.category)
                {
                    n++;
                    if (n == activeSubject - 1)
                    {
                        char buf[4] = "   ";
                        strncpy(buf, getSubjectName(subjectRAMs[i].index), 3);
                        buf[3] = '\0';
                        lcd1.print(buf);
                        break;
                    }
                }
            }
        }

        lcd1.setCursor(13, 1);
        if (activeSubject < maxSlot)
        {
            byte n = 0;
            for (byte i = 0; i < subjectCount; i++)
            {
                if (subjectRAMs[i].category == newSchedule.category)
                {
                    n++;
                    if (n == activeSubject + 1)
                    {
                        char buf[4] = "   ";
                        strncpy(buf, getSubjectName(subjectRAMs[i].index), 3);
                        buf[3] = '\0';
                        lcd1.print(buf);
                        break;
                    }
                }
            }
        }
        else
        {
            lcd1.print("   ");
        }

        lastActiveSubject = activeSubject;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 3: Interval
// ─────────────────────────────────────────────────────────────────────────────
static void intervalSelection()
{
    static byte activeInterval = 0;
    static byte lastActiveInterval = 0xFF;

    if (rightPressed())
        activeInterval = (activeInterval < 3) ? activeInterval + 1 : 0;
    else if (leftPressed())
        activeInterval = (activeInterval > 0) ? activeInterval - 1 : 3;
    else if (enterPressed())
    {
        newSchedule.interval = activeInterval;
        currentStep = TIME_INPUT;
    }

    if (lastActiveInterval != activeInterval)
    {
        static const char labels[4][7] = {"Once", "Daily", "Weekly", "Yearly"};
        lcd1.clear();
        for (byte i = 0; i < 4; i++)
        {
            byte col = (i % 2 == 0) ? 0 : 9;
            byte row = i / 2;
            lcd1.setCursor(col, row);
            lcd1.print(activeInterval == i ? ">" : " ");
            lcd1.print(labels[i]);
        }
        lastActiveInterval = activeInterval;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 4: Time input
// ─────────────────────────────────────────────────────────────────────────────
static void timeInput()
{
    static byte hour = 0;
    static byte minute = 0;
    static byte day = 1;
    static byte month = 1;
    static byte dow = 0;
    static int8_t cursor = 0;
    static bool needRender = true;
    static bool initialised = false;

    if (!initialised)
    {
        hour = nowTime.hour();
        minute = nowTime.minute();
        day = nowTime.day();
        month = nowTime.month();
        dow = nowTime.dayOfTheWeek();
        cursor = (newSchedule.interval == WEEKLY) ? -1 : 0;
        needRender = true;
        initialised = true;
    }

    byte maxCursor;
    switch (newSchedule.interval)
    {
    case DAILY:
        maxCursor = 1;
        break;
    case WEEKLY:
        maxCursor = 1;
        break;
    case ONCE_ONLY:
    case YEARLY:
        maxCursor = 3;
        break;
    default:
        maxCursor = 1;
        break;
    }

    if (rightPressed())
    {
        switch (cursor)
        {
        case -1:
            dow = (dow + 1) % 7;
            break;
        case 0:
            hour = (hour + 1) % 24;
            break;
        case 1:
            minute = (minute + 1) % 60;
            break;
        case 2:
            day = (day % 31) + 1;
            break;
        case 3:
            month = (month % 12) + 1;
            break;
        }
        needRender = true;
    }
    else if (leftPressed())
    {
        switch (cursor)
        {
        case -1:
            dow = (dow + 6) % 7;
            break;
        case 0:
            hour = (hour + 23) % 24;
            break;
        case 1:
            minute = (minute + 59) % 60;
            break;
        case 2:
            day = (day > 1) ? day - 1 : 31;
            break;
        case 3:
            month = (month > 1) ? month - 1 : 12;
            break;
        }
        needRender = true;
    }
    else if (enterPressed())
    {
        if (cursor < (int8_t)maxCursor)
        {
            cursor++;
        }
        else
        {
            // FIX: removed the dead "cursor > maxCursor → show OK" branch;
            // on the final field we go directly to the next step.
            newSchedule.hour = hour;
            newSchedule.minute = minute;
            newSchedule.day = day;
            newSchedule.month = month;
            if (newSchedule.interval == WEEKLY)
                newSchedule.day = dow + 1;
            initialised = false;
            currentStep = SELECT_REMINDER;
        }
        needRender = true;
    }
    else if (removePressed())
    {
        int8_t minC = (newSchedule.interval == WEEKLY) ? -1 : 0;
        if (cursor > minC)
        {
            cursor--;
            needRender = true;
        }
    }

    if (!needRender)
        return;
    needRender = false;

    char buf[17];
    static const char *dowNames[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    lcd1.setCursor(0, 0);
    lcd1.print("                ");

    switch (newSchedule.interval)
    {
    case DAILY:
        snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
        printCentered(buf, 0);
        break;
    case WEEKLY:
        snprintf(buf, sizeof(buf), "%s %02d:%02d", dowNames[dow], hour, minute);
        printCentered(buf, 0);
        break;
    default:
        snprintf(buf, sizeof(buf), "%02d:%02d %02d-%02d", hour, minute, day, month);
        printCentered(buf, 0);
        break;
    }

    lcd1.setCursor(0, 1);
    lcd1.print("                ");

    switch (newSchedule.interval)
    {
    case DAILY:
    {
        int base = (16 - 5) / 2;
        for (int8_t f = 0; f <= 1; f++)
        {
            lcd1.setCursor(base + f * 3, 1);
            lcd1.print(cursor == f ? "^^" : "  ");
        }
        break;
    }
    case WEEKLY:
    {
        int base = (16 - 9) / 2;
        int cols[3] = {base, base + 4, base + 7};
        for (int8_t f = -1; f <= 1; f++)
        {
            lcd1.setCursor(cols[f + 1], 1);
            lcd1.print(cursor == f ? "^^" : "  ");
        }
        break;
    }
    default:
    {
        int base = (16 - 11) / 2;
        int cols[4] = {base, base + 3, base + 6, base + 9};
        for (int8_t f = 0; f <= 3; f++)
        {
            lcd1.setCursor(cols[f], 1);
            lcd1.print(cursor == f ? "^^" : "  ");
        }
        break;
    }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 5: Reminder
// ─────────────────────────────────────────────────────────────────────────────
static void reminderSelection()
{
    static byte activeReminder = 0;
    static byte lastActiveReminder = 0xFF;
    static bool headerPrinted = false;

    if (!headerPrinted)
    {
        lcd1.clear();
        printCentered("Notify Me When", 0);
        headerPrinted = true;
    }

    if (rightPressed())
        activeReminder = (activeReminder < 2) ? activeReminder + 1 : 0;
    else if (leftPressed())
        activeReminder = (activeReminder > 0) ? activeReminder - 1 : 2;
    else if (enterPressed())
    {
        newSchedule.flags = activeReminder + 1;
        headerPrinted = false;
        currentStep = CONFIRMATION;
    }

    if (lastActiveReminder != activeReminder)
    {
        static const char *opts[3] = {"On The Time", "- 1 Hour", "- 1 Day"};
        printCentered(opts[activeReminder], 1);
        lastActiveReminder = activeReminder;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP 6: Confirmation
// ─────────────────────────────────────────────────────────────────────────────
static void confirmation()
{
    static bool initialised = false;
    static bool dipState = false;
    static byte outcome = 0;
    static unsigned long lastDip = 0;

    static const char *dowNames[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    if (!initialised)
    {
        lcd1.clear();
        char buf[17];
        switch (newSchedule.interval)
        {
        case ONCE_ONLY:
            snprintf(buf, sizeof(buf), "At %02d:%02d %02d-%02d",
                     newSchedule.hour, newSchedule.minute,
                     newSchedule.day, newSchedule.month);
            break;
        case DAILY:
            snprintf(buf, sizeof(buf), "Evry Day %02d:%02d",
                     newSchedule.hour, newSchedule.minute);
            break;
        case WEEKLY:
        {
            byte d = (newSchedule.day >= 1 && newSchedule.day <= 7)
                         ? newSchedule.day - 1
                         : 0;
            snprintf(buf, sizeof(buf), "Evry %s %02d:%02d",
                     dowNames[d], newSchedule.hour, newSchedule.minute);
            break;
        }
        case YEARLY:
            snprintf(buf, sizeof(buf), "Yrly %02d:%02d %02d-%02d",
                     newSchedule.hour, newSchedule.minute,
                     newSchedule.day, newSchedule.month);
            break;
        }
        lcd1.setCursor(0, 0);
        lcd1.print(buf);
        initialised = true;
        outcome = 0;
        dipState = false;
        lastDip = millis();
    }

    if (outcome == 0)
    {
        if (enterPressed())
        {
            addSchedule(newSchedule.hour, newSchedule.minute,
                        newSchedule.day, newSchedule.month,
                        newSchedule.category, newSchedule.subject,
                        newSchedule.flags, newSchedule.interval);
            outcome = 1;
            lcd1.clear();
        }
        if (removePressed())
        {
            outcome = 2;
            lcd1.clear();
        }
    }

    unsigned long now = millis();
    if (now - lastDip >= 1000)
    {
        lastDip = now;
        dipState = !dipState;

        if (outcome == 0)
        {
            lcd1.setCursor(0, 1);
            if (dipState)
            {
                // FIX: replaced "lcd1.print("" + String(subName))" with a plain
                // char buffer to avoid Arduino String heap allocation on 2 KB SRAM.
                char subName[14] = "";
                byte subId = 0;
                for (byte i = 0; i < subjectCount; i++)
                {
                    if (subjectRAMs[i].subjectId == newSchedule.subject)
                    {
                        strncpy(subName, getSubjectName(subjectRAMs[i].index), 13);
                        subName[13] = '\0';
                        subId = subjectRAMs[i].category; // use category as icon index
                        break;
                    }
                }
                lcd1.write(subId);
                lcd1.print(" ");
                lcd1.print(subName);
            }
            else
            {
                lcd1.print(">OK      XCancel");
            }
        }
        else if (outcome == 1)
        {
            if (dipState)
            {
                printCentered("Schedule Added!", 0);
            }
            else
            {
                initialised = false;
                // FIX: also reset the whole flow so re-entry starts clean.
                flowInitialised = false;
                currentState = MENU;
            }
        }
        else
        {
            if (dipState)
            {
                printCentered("Cancelled!", 0);
            }
            else
            {
                initialised = false;
                flowInitialised = false;
                currentState = MENU;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Top-level dispatcher
// ─────────────────────────────────────────────────────────────────────────────
void addScheduleInterface()
{
    // FIX: reset all flow state whenever we enter ADD_SCHEDULE fresh.
    // Without this, returning after cancelling resumes mid-flow with stale data.
    if (!flowInitialised)
    {
        currentStep = SELECT_CATEGORY;
        lastStep = (Step)0xFF;
        stepJustChanged = false;
        memset(&newSchedule, 0, sizeof(newSchedule));
        flowInitialised = true;
    }

    // ── Step transition banner ────────────────────────────────────────────────
    if (currentStep != lastStep)
    {
        lastStep = currentStep;
        stepJustChanged = true;

        lcd1.clear();
        printCentered(stepTitle(currentStep), 0);
        return;
    }

    if (stepJustChanged)
    {
        stepJustChanged = false;
        lcd1.clear();
        return;
    }

    // ── Dispatch ─────────────────────────────────────────────────────────────
    switch (currentStep)
    {
    case SELECT_CATEGORY:
        categorySelection();
        break;
    case SELECT_SUBJECT:
        subjectSelection();
        break;
    case SELECT_INTERVAL:
        intervalSelection();
        break;
    case TIME_INPUT:
        timeInput();
        break;
    case SELECT_REMINDER:
        reminderSelection();
        break;
    case CONFIRMATION:
        confirmation();
        break;
    default:
        break;
    }
}