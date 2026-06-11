#include <Arduino.h>
#include "data.h"

SubjectRAM subjectRAMs[SUBJECT_MAX_RAM];
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
    memset(subjectRAMs, 0, sizeof(subjectRAMs));

    for (int i = 0; i < SUBJECT_MAX_EEPROM && subjectCount < SUBJECT_MAX_RAM; i++)
    {
        byte cat = readEEPROM(subjectAddr(1, i));

        if (cat == 0xFF)
            break; // end of list sentinel
        if (cat == 0xFE)
            continue; // deleted slot

        subjectRAMs[subjectCount].category = cat;
        subjectRAMs[subjectCount].subjectId = readEEPROM(subjectAddr(2, i));
        subjectRAMs[subjectCount].index = i;
        subjectCount++;
    }
}

char *getSubjectName(byte index)
{
    static char buffer[15]; // Adjust size as needed
    memset(buffer, 0, sizeof(buffer));

    for (size_t j = 0; j < sizeof(buffer) - 1; j++)
    {
        byte c = readEEPROM(subjectAddr(3 + j, index));
        buffer[j] = (c == 0) ? '\0' : c; // null-terminate on 0
        if (c == 0)
            break;
    }
    buffer[sizeof(buffer) - 1] = '\0'; // ensure null-termination
    return buffer;
}