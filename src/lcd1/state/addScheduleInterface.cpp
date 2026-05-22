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

struct tempSchedule
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

tempSchedule newSchedule;

enum Step
{
    SELECT_CATEGORY,
    SELECT_SUBJECT,
    SELECT_INTERVAL,
    TIME_INPUT,
    SELECT_REMINDER,
    CONFIRMATION
};

byte currentStep = SELECT_CATEGORY;

//=============================================================================
// Category Selection

void categoryDisplayUpdate(int activeCategoryRow)
{
    // Category Name
    lcd1.setCursor(0, 0);
    lcd1.print("              "); // Clear the line

    delay(5); // Debounce delay

    switch (activeCategoryRow)
    {
    case HOMEWORK:
        lcd1.print("> Homework");
        break;
    case EXAM:
        lcd1.print("> Exam");
        break;
    case PERSONAL:
        lcd1.print("> Personal");
        break;
    case OTHER:
        lcd1.print("> Other");
        break;
    }

    // Logika untuk menampilkan pilihan kategori
    for (int i = 0; i < 4; i++)
    {
        lcd1.setCursor(4 + (i * 2), 1);
        if (activeCategoryRow == i)
        {
            lcd1.print(">");
        }
        else
        {
            lcd1.print(" ");
        }
        lcd1.print(byte(i));
    }
}

void categorySelection()
{
    static int activeCategoryRow = HOMEWORK;
    static int lastActiveCategoryRow = -1;

    if (lastActiveCategoryRow != activeCategoryRow)
    {
        categoryDisplayUpdate(activeCategoryRow);
        lastActiveCategoryRow = activeCategoryRow;
    }

    // Logika untuk memilih kategori dengan tombol

    // right button
    if (rightPressed())
    {
        if (activeCategoryRow < OTHER)
        {
            activeCategoryRow++;
        }
        else
        {
            activeCategoryRow = HOMEWORK;
        }
    }

    // left button
    else if (leftPressed())
    {
        if (activeCategoryRow > HOMEWORK)
        {
            activeCategoryRow--;
        }
        else
        {
            activeCategoryRow = OTHER;
        }
    }

    // enter button
    else if (enterPressed())
    {
        newSchedule.category = activeCategoryRow;
        // Pindah ke langkah berikutnya, misalnya memilih subjek
        // currentStep = SELECT_SUBJECT;
        delay(5); // Debounce delay
        currentStep = SELECT_SUBJECT;
    }
}

//======================================================================================
// Subject Selection

byte activeSubject = 0;
byte lastActiveSubject = -1;
char prevSubject[14];
Subject currentSubject;
char nextSubject[14];

bool newSubject()
{
    char *name = typing();

    if (strlen(name) == 0)
    {
        return false;
    }

    Subject moreSubject;
    moreSubject.category = newSchedule.category;

    bool idTaken[50] = {false}; // Tracks which IDs 0-49 are used
    byte subjectId = 255;       // Default value

    // Mark all currently used IDs as true
    for (int i = 0; i < 50; i++)
    {
        if (subjects[i].subjectId >= 0 && subjects[i].subjectId < 50)
        {
            idTaken[subjects[i].subjectId] = true;
        }
    }

    // Find the first ID that remains false
    for (int i = 0; i < 50; i++)
    {
        if (!idTaken[i])
        {
            subjectId = i;
            break; // Found an available ID
        }
    }

    moreSubject.subjectId = subjectId;
    strcpy(moreSubject.subject, name);

    addSubject(moreSubject.category, moreSubject.subjectId, moreSubject.subject);

    return true;
}

void subjectDisplayUpdate()
{
    // Logika untuk memperbarui tampilan subjek berdasarkan activeSubject
    int subjectFound = 0;
    for (unsigned int i = 0; i < sizeof(subjects) / sizeof(subjects[0]); i++)
    {
        if (subjects[i].category == newSchedule.category)
        {
            if (activeSubject == 0)
            {
                strcpy(prevSubject, "   ");
                strcpy(currentSubject.subject, "Add Subject");
                strcpy(nextSubject, subjects[i].subject);

                break;
            }

            subjectFound += 1;
            if (subjectFound == activeSubject)
            {
                currentSubject = subjects[i];
                if (activeSubject == 1)
                {
                    strcpy(prevSubject, "Add");
                }
            }
            else if (subjectFound == activeSubject - 1)
            {
                strcpy(prevSubject, subjects[i].subject);
            }
            else if (subjectFound == activeSubject + 1)
            {
                if (strlen(subjects[i].subject) != 0)
                {
                    strcpy(nextSubject, subjects[i].subject);
                }
                else
                {
                    strcpy(nextSubject, "   ");
                }
                break;
            }
        }
    }

    // middle
    char middlePosition = (16 - strlen(currentSubject.subject)) / 2;
    lcd1.setCursor(middlePosition, 0);
    lcd1.print(currentSubject.subject);

    // left
    char leftName[14];
    int len = strlen(prevSubject);
    strlcpy(leftName, prevSubject + len - 3, 3);
    leftName[3] = '\0'; // Pastikan string berakhir dengan null terminator
    lcd1.setCursor(0, 1);
    lcd1.print(".." + String(leftName));

    // right
    char rightName[14];
    strncpy(rightName, nextSubject, 3);
    rightName[3] = '\0'; // Pastikan string berakhir dengan null terminator
    lcd1.setCursor(16 - 5, 1);
    lcd1.print(String(rightName) + "..");
}

void subjectSelection()
{
    static bool isAddSubject = false;

    if (isAddSubject)
    {
        bool addSubjectStatus = newSubject();
        if (addSubjectStatus)
        {
            isAddSubject = false;
            return;
        }
        else
        {
            return;
        }
    }

    // left button
    if (leftPressed())
    {
        if (activeSubject > 0)
        {
            activeSubject--;
        }
        else
        {
            activeSubject = 0;
        }
    }

    // right button
    else if (rightPressed())
    {
        if (nextSubject[0] != '\0')
        {
            activeSubject++;
        }
        else
        {
            activeSubject = activeSubject;
        }
    }

    // enter button
    else if (enterPressed())
    {
        if (activeSubject == 0)
        {
            // Logika untuk menambahkan subjek baru
            isAddSubject = true;
            return;
        }
        else
        {
            // Logika untuk memilih subjek yang sudah ada
            newSchedule.subject = currentSubject.subjectId;
            // Pindah ke langkah berikutnya, misalnya memilih interval
            currentStep = SELECT_INTERVAL;
        }
        delay(5); // Debounce delay
    }

    if (lastActiveSubject != activeSubject)
    {
        lcd1.clear();

        delay(5); // Debounce delay

        subjectDisplayUpdate();
        lastActiveSubject = activeSubject;
    }
}

//======================================================================================
// Interval Selection

const char intervals[4][10] = {
    "Once",
    "Daily",
    "Weekly",
    "Yearly"};

byte activeInterval = 0;
byte lastActiveInterval = -1;

void updateDisplayInterval()
{
    byte writeCol = 0;
    for (int i = 0; i < 4; i++)
    {
        if (i % 2 == 0)
        {
            lcd1.setCursor(7 - strlen(intervals[i]), writeCol);
            lcd1.print(intervals[i]);
            if (activeInterval == i)
            {
                lcd1.print("<");
            }
            else
            {
                lcd1.print(" ");
            }
        }
        else
        {
            lcd1.setCursor(9, writeCol);
            if (activeInterval == i)
            {
                lcd1.print(">");
            }
            else
            {
                lcd1.print(" ");
            }

            lcd1.print(intervals[i]);

            writeCol++;
        }
    }
}

void intervalSelection()
{
    // right button
    if (rightPressed())
    {
        if (activeInterval < 3)
        {
            activeInterval++;
        }
        else
        {
            activeInterval = 0;
        }
    }

    // left button
    else if (leftPressed())
    {
        if (activeInterval > 0)
        {
            activeInterval--;
        }
        else
        {
            activeInterval = 3;
        }
    }

    // enter button
    else if (enterPressed())
    {
        newSchedule.interval = activeInterval;
        // Pindah ke langkah berikutnya, misalnya memilih jam dan menit
        currentStep = TIME_INPUT;
        delay(5); // Debounce delay
    }

    if (lastActiveInterval != activeInterval)
    {
        updateDisplayInterval();
        lastActiveInterval = activeInterval;
    }
}

//======================================================================================
// Time Input

byte hour = nowTime.hour();
byte minute = nowTime.minute();
byte day = nowTime.day();
byte month = nowTime.month();
byte dayOfWeek = nowTime.dayOfTheWeek();
const char *dayNames[7] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"};

bool changeHour = true;
signed char timeInputCursor = newSchedule.interval == WEEKLY ? -1 : 0; // -1 untuk memilih hari dalam seminggu, 0 untuk jam, 1 untuk menit, 2 untuk tanggal, 3 untuk bulan
byte lastTimeInputCursor = -2;

byte getMaxTimeInputFields()
{
    // Daily: 2 fields (hour, minute)
    // Weekly: 3 fields (hour, minute, day of week)
    // Once: 4 fields (hour, minute, day, month)
    // Yearly: 4 fields (hour, minute, day, month)
    if (newSchedule.interval == 0 || newSchedule.interval == 1)
    {
        return 2;
    }
    else if (newSchedule.interval == 2)
    {
        return 3;
    }
    else
    {
        return 4;
    }
}

void updateTimeDisplay()
{
    lcd1.clear();

    byte maxFields = getMaxTimeInputFields();

    if (maxFields == 2)
    {
        // Daily: only show hour and minute
        lcd1.setCursor((16 - 5) / 2, 0);
        String display = (hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);
        lcd1.print(display);
    }
    else if (maxFields == 3)
    {
        // Weekly: show day of week, hour, and minute
        lcd1.setCursor((16 - 8) / 2, 0);
        String display = String(dayNames[dayOfWeek]) + " " + (hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);
        lcd1.print(display);
    }
    else
    {
        // Once or Yearly: show full date and time
        lcd1.setCursor((16 - 12) / 2, 0);
        String display = (hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + " " + (day < 10 ? "0" : "") + String(day) + "-" + (month < 10 ? "0" : "") + String(month);
        lcd1.print(display);
    }
}

void updateCursorTime()
{
    byte maxFields = getMaxTimeInputFields();

    if (maxFields == 2)
    {
        // Daily: show cursor for hour and minute
        for (int i = 0; i < 2; i++)
        {
            lcd1.setCursor((16 - 5) / 2 + i * 3, 1);
            if (timeInputCursor == i)
            {
                lcd1.print("^^");
            }
            else
            {
                lcd1.print("  ");
            }
        }

        if (timeInputCursor == 2)
        {
            lcd1.setCursor(16 - 2, 1);
            lcd1.print("OK");
        }
    }
    else if (maxFields == 3)
    {
        // Weekly: show cursor for hour, minute, and day of week
        for (int i = 0; i < 3; i++)
        {
            lcd1.setCursor(((16 - 8) / 2) + 1 + i * 3, 1);
            if (timeInputCursor == i)
            {
                lcd1.print("^^");
            }
            else
            {
                lcd1.print("  ");
            }
        }

        if (timeInputCursor == 3)
        {
            lcd1.setCursor(16 - 2, 1);
            lcd1.print("OK");
        }
    }
    else
    {
        // Once or Yearly: show cursor for all four fields
        for (int i = 0; i < 4; i++)
        {
            lcd1.setCursor((16 - 12) / 2 + i * 3, 1);
            if (timeInputCursor == i)
            {
                lcd1.print("^^");
            }
            else
            {
                lcd1.print("  ");
            }
        }

        if (timeInputCursor == 4)
        {
            lcd1.setCursor(16 - 2, 1);
            lcd1.print("OK");
        }
    }
}

void timeInput()
{
    if (changeHour)
    {
        updateTimeDisplay();
        changeHour = false;
    }

    if (lastTimeInputCursor != timeInputCursor)
    {
        updateCursorTime();
        lastTimeInputCursor = timeInputCursor;
    }

    byte maxFields = getMaxTimeInputFields();

    // right button
    if (rightPressed())
    {
        switch (timeInputCursor)
        {
        case -1:
            dayOfWeek = ((dayOfWeek + 1) % 7) + 1;
            break;
        case 0:
            hour = (hour + 1) % 24;
            break;
        case 1:
            minute = (minute + 1) % 60;
            break;
        case 2:
            day = ((day + 1) % 31) + 1;
            break;
        case 3:
            month = ((month + 1) % 12) + 1;
            break;
        }
    }

    else if (leftPressed())
    {
        switch (timeInputCursor)
        {
        case -1:
            dayOfWeek = ((dayOfWeek + 5) % 7) + 1;
            break;
        case 0:
            hour = (hour + 23) % 24;
            break;
        case 1:
            minute = (minute + 59) % 60;
            break;
        case 2:
            day = ((day + 29) % 31) + 1;
            break;
        case 3:
            month = ((month + 10) % 12) + 1;
            break;
        }
    }

    else if (enterPressed())
    {
        if (timeInputCursor < maxFields)
        {
            timeInputCursor++;
        }
        else
        {
            // Simpan jadwal baru ke dalam array todaySchedules
            newSchedule.hour = hour;
            newSchedule.minute = minute;
            newSchedule.day = day;
            newSchedule.month = month;

            // Pindah ke langkah berikutnya, misalnya memilih pengingat
            currentStep = SELECT_REMINDER;
        }
        delay(5); // Debounce delay
    }

    else if (removePressed())
    {
        if (timeInputCursor > 0)
        {
            timeInputCursor--;
        }
    }
}

//======================================================================================
// Reminder Selection

const char *reminderOptions[4] = {
    "On The Time",
    "- 1 Hour",
    "- 1 Day"};

byte activeReminder = 0;
byte lastActiveReminder = -1;

bool initialDisplayReminder = false;

void updateDisplayReminder()
{
    lcd1.clear();

    lcd1.setCursor((16 - strlen(reminderOptions[activeReminder])) / 2, 1);
    lcd1.print(reminderOptions[activeReminder]);
}

void reminderSelection()
{
    if (!initialDisplayReminder)
    {
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("Notify Me When");
        initialDisplayReminder = true;
    }

    // right button
    if (rightPressed())
    {
        if (activeReminder < 2)
        {
            activeReminder++;
        }
        else
        {
            activeReminder = 0;
        }
        delay(200); // Debounce delay
    }

    // left button
    else if (leftPressed())
    {
        if (activeReminder > 0)
        {
            activeReminder--;
        }
        else
        {
            activeReminder = 2;
        }
        delay(200); // Debounce delay
    }

    // enter button
    else if (enterPressed())
    {
        newSchedule.flags = activeReminder + 1; // Sesuaikan dengan bit yang digunakan untuk reminder
        // Pindah ke langkah berikutnya, misalnya konfirmasi jadwal
        currentStep = CONFIRMATION;
        delay(5); // Debounce delay
    }

    if (lastActiveReminder != activeReminder)
    {
        updateDisplayReminder();
        lastActiveReminder = activeReminder;
    }
}

//======================================================================================
// Confirmation

bool initialDisplayConfirmation = false;
bool confirmationDip = false;

void confirmation()
{
    if (!initialDisplayConfirmation)
    {
        lcd1.clear();
        lcd1.setCursor(0, 0);
        switch (newSchedule.interval)
        {
        case 0:
            lcd1.print("At " + String(hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + " On " + (day < 10 ? "0" : "") + String(day) + "-" + (month < 10 ? "0" : "") + String(month));
            break;

        case 1:
            lcd1.print("Every Day " + String(hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute));
            break;
        case 2:
            lcd1.print("Every " + String(dayNames[newSchedule.day]) + " " + String(hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute));
            break;
        case 3:
            lcd1.print("Yearly At " + String(hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + " On " + (day < 10 ? "0" : "") + String(day) + "-" + (month < 10 ? "0" : "") + String(month));
            break;
        }

        initialDisplayConfirmation = true;
    }

    static bool dipStatus = false;
    char subject[14] = "";

    for (int i = 0; i < 50; i++)
    {
        if (subjects[i].subjectId == newSchedule.subject)
        {
            strcpy(subject, subjects[i].subject);
            break;
        }
    }

    if (blinkLogic(1000))
    {
        dipStatus = !dipStatus;
        if (dipStatus)
        {
            lcd1.setCursor(0, 1);
            lcd1.print(byte(newSchedule.category));
            lcd1.print(" " + String(subject));
        }
        else
        {
            lcd1.print("> OK   X Cancel");
        }
    }

    static byte lastDisplay = 0; // 1: Added, 2: Cancelled

    if (enterPressed() && lastDisplay == 0)
    {
        addSchedule(newSchedule.hour, newSchedule.minute, newSchedule.day, newSchedule.month, newSchedule.category, newSchedule.subject, newSchedule.flags, newSchedule.interval);
        lastDisplay = 1;
    }

    if (removePressed() && lastDisplay == 0)
    {
        lastDisplay = 2;
    }

    if (lastDisplay == 1)
    {
        if (blinkLogic(1000))
        {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("New Schedule");
            lcd1.setCursor(0, 1);
            lcd1.print("Added!");
        }
        else
        {
            currentState = MENU;
            return;
        }
    }
    else if (lastDisplay == 2)
    {
        if (blinkLogic(1000))
        {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print("Adding Schedule");
            lcd1.setCursor(0, 1);
            lcd1.print("Cancelled!");
        }
        else
        {
            currentState = MENU;
            return;
        }
    }
}

void addScheduleInterface()
{
    static byte last = SELECT_INTERVAL;

    if (last != currentStep)
    {
        lcd1.clear();
        if (blinkLogic(500))
        {
            switch (currentStep)
            {
            case SELECT_CATEGORY:
                lcd1.setCursor((16 - 6) / 2, 0);
                lcd1.print("Select");
                lcd1.setCursor((16 - 9) / 2, 1);
                lcd1.print("Category!");
                break;
            case SELECT_SUBJECT:
                lcd1.setCursor((16 - 6) / 2, 0);
                lcd1.print("Select");
                lcd1.setCursor((16 - 8) / 2, 1);
                lcd1.print("Subject!");
                break;
            case SELECT_INTERVAL:
                lcd1.setCursor((16 - 6) / 2, 0);
                lcd1.print("Select");
                lcd1.setCursor((16 - 9) / 2, 1);
                lcd1.print("Interval!");
                break;
            case TIME_INPUT:
                lcd1.setCursor((16 - 5) / 2, 0);
                lcd1.print("Input");
                lcd1.setCursor((16 - 11) / 2, 1);
                lcd1.print("Time Date!");
                break;
            case SELECT_REMINDER:
                lcd1.setCursor((16 - 6) / 2, 0);
                lcd1.print("Select");
                lcd1.setCursor((16 - 10) / 2, 1);
                lcd1.print("Reminder!");
                break;
            case CONFIRMATION:
                lcd1.setCursor((16 - 8) / 2, 0);
                lcd1.print("Confirm");
                lcd1.setCursor((16 - 3) / 2, 1);
                lcd1.print("It!");
                break;

            default:
                break;
            }
            return;
        }
        else
        {
            last = currentStep;
            lcd1.clear();
            return;
        }
    }

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