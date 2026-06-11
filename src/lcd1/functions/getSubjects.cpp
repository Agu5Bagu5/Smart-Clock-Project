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
    static char buffer[15];
    memset(buffer, 0, sizeof(buffer));

    // FIX: was subjectAddr(3 + j, index) which computes SUBJECT_BASE + 16*index + (3+j).
    // That happens to be numerically identical to subjectAddr(3, index) + j, but only
    // because subjectAddr is linear.  Write it unambiguously to match addSubject.cpp,
    // which writes chars at subjectAddr(3, i) + e.
    int base = subjectAddr(3, index);
    for (size_t j = 0; j < sizeof(buffer) - 1; j++)
    {
        byte c = readEEPROM(base + j);
        if (c == 0)
        {
            buffer[j] = '\0';
            break;
        }
        buffer[j] = (char)c;
    }
    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}