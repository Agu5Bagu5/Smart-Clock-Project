#include <Arduino.h>
#include "data.h"

Subject subjects[SUBJECT_MAX_RAM];
byte subjectCount = 0;

static int subjectAddr(int offset, int index)
{
    return SUBJECT_BASE + SUBJECT_SLOT * index + offset;
}

void getSubjects()
{
    // FIX: RAMIndex was static, so repeated calls kept appending
    // instead of refreshing. Use a plain local counter and reset
    // the globals at the top of every call.
    subjectCount = 0;
    memset(subjects, 0, sizeof(subjects));

    for (int i = 0; i < SUBJECT_MAX_EEPROM && subjectCount < SUBJECT_MAX_RAM; i++)
    {
        byte cat = readEEPROM(subjectAddr(1, i));

        if (cat == 0xFF)
            break; // end of list sentinel
        if (cat == 0xFE)
            continue; // deleted slot

        subjects[subjectCount].category = cat;
        subjects[subjectCount].subjectId = readEEPROM(subjectAddr(2, i));
        for (int e = 0; e < 14; e++)
        {
            subjects[subjectCount].subject[e] = (char)readEEPROM(subjectAddr(3, i) + e);
        }
        subjects[subjectCount].subject[13] = '\0'; // guarantee null-termination
        subjectCount++;
    }
}