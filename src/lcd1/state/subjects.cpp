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

enum SubjectMode
{
    CATEGORY_SELECT,
    SUBJECT_LIST,
    DELETE_CONFIRM
};

void reloadVisibleSubjects(
    byte selectedCategory,
    byte visibleSubjects[],
    byte &visibleCount)
{
    visibleCount = 0;

    for (byte i = 0; i < subjectCount; i++)
    {
        if (selectedCategory == 0 || subjectRAMs[i].category == selectedCategory)
        {
            visibleSubjects[visibleCount++] = i;
        }
    }
}

void subjects()
{
    static bool initialized = false;

    static SubjectMode mode;

    static byte selectedCategory = 0;

    static byte visibleSubjects[SUBJECT_MAX_RAM];
    static byte visibleCount = 0;

    static byte currentSubject = 0;
    static bool deleteYes = false;

    static bool forceRender = true;

    const char *categories[] =
        {
            "All",
            "Academic",
            "Task",
            "Event",
            "Personal",
            "Other"};

    const byte categoryCount = 6;

    // =====================================================
    // FIRST ENTRY
    // =====================================================

    if (!initialized)
    {
        deleteYes = false;
        selectedCategory = 0;

        mode = CATEGORY_SELECT;

        visibleCount = 0;
        currentSubject = 0;

        lcd1.clear();

        forceRender = true;
        initialized = true;
    }

    // =====================================================
    // CATEGORY SELECT MODE
    // =====================================================

    if (mode == CATEGORY_SELECT)
    {
        if (leftPressed())
        {
            if (selectedCategory == 0)
                selectedCategory = categoryCount - 1;
            else
                selectedCategory--;

            forceRender = true;
        }

        if (rightPressed())
        {
            selectedCategory++;

            if (selectedCategory >= categoryCount)
                selectedCategory = 0;

            forceRender = true;
        }

        if (enterPressed())
        {
            reloadVisibleSubjects(
                selectedCategory,
                visibleSubjects,
                visibleCount);

            currentSubject = 0;

            mode = SUBJECT_LIST;

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
            lcd1.print("Category");

            lcd1.setCursor(0, 1);
            lcd1.print(categories[selectedCategory]);

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
            mode = SUBJECT_LIST;

            lcd1.clear();

            forceRender = true;

            return;
        }

        if (enterPressed())
        {
            if (!deleteYes)
            {
                mode = SUBJECT_LIST;

                lcd1.clear();

                forceRender = true;

                return;
            }

            // FIX: use subjectRAMs[subjectIndex].index (the actual EEPROM slot),
            // not subjectId (an arbitrary assigned byte).  Also write to offset +1
            // (the category field) with value 0xFE (deleted marker), not offset 0
            // (unused padding) with 0xFF (end-sentinel).
            byte subjectIndex = visibleSubjects[currentSubject];
            int eepromSlot = subjectRAMs[subjectIndex].index;
            int base = SUBJECT_BASE + eepromSlot * SUBJECT_SLOT;

            writeEEPROM(base + 1, 0xFE); // mark category field as deleted

            reloadVisibleSubjects(
                selectedCategory,
                visibleSubjects,
                visibleCount);

            if (visibleCount == 0)
            {
                currentSubject = 0;
            }
            else if (currentSubject >= visibleCount)
            {
                currentSubject = visibleCount - 1;
            }

            getSubjects();

            mode = SUBJECT_LIST;

            lcd1.clear();

            forceRender = true;

            return;
        }

        if (!forceRender)
            return;

        lcd1.clear();

        lcd1.setCursor(0, 0);
        lcd1.print("Delete Subject?");

        lcd1.setCursor(0, 1);

        if (!deleteYes)
        {
            lcd1.print(">No     Yes");
        }
        else
        {
            lcd1.print(" No    >Yes");
        }

        forceRender = false;

        return;
    }

    // =====================================================
    // SUBJECT LIST MODE
    // =====================================================

    if (leftPressed())
    {
        if (visibleCount > 0)
        {
            if (currentSubject == 0)
                currentSubject = visibleCount - 1;
            else
                currentSubject--;

            forceRender = true;
        }
    }

    if (rightPressed())
    {
        if (visibleCount > 0)
        {
            currentSubject++;

            if (currentSubject >= visibleCount)
                currentSubject = 0;

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
        mode = CATEGORY_SELECT;

        lcd1.clear();

        forceRender = true;

        return;
    }

    if (!forceRender)
        return;

    lcd1.clear();

    // =====================================================
    // NO SUBJECTS
    // =====================================================

    if (visibleCount == 0)
    {
        lcd1.setCursor(0, 0);
        lcd1.print("Subjects");

        lcd1.setCursor(0, 1);
        lcd1.print("No subjects");

        forceRender = false;
        return;
    }

    // =====================================================
    // DISPLAY SUBJECT
    // =====================================================

    byte subjectIndex = visibleSubjects[currentSubject];

    char header[17];

    sprintf(
        header,
        "%d/%d",
        currentSubject + 1,
        visibleCount);

    lcd1.setCursor(0, 0);
    lcd1.print(header);

    lcd1.setCursor(0, 1);

    lcd1.write(subjectRAMs[subjectIndex].category);
    lcd1.print(" ");

    lcd1.print(getSubjectName(subjectRAMs[subjectIndex].index));

    forceRender = false;
}