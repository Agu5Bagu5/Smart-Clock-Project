#include <Arduino.h>
#include <Wire.h>
#include "data.h"

/*  Subject slot layout (16 bytes, starting at address 0):
 *   offset 0  : (padding / unused)
 *   offset 1  : category  — 0xFF = end-sentinel, 0xFE = deleted
 *   offset 2  : subjectId
 *   offset 3…16 : name (14 bytes, null-terminated)
 */
static int subjectAddr(int offset, int index)
{
    return SUBJECT_BASE + SUBJECT_SLOT * index + offset;
}

void addSubject(byte category, byte subjectId, const char *subject)
{
    for (int i = 0; i < SUBJECT_MAX_EEPROM; i++)
    {
        byte cat = readEEPROM(subjectAddr(1, i));

        if (cat == 0xFF || cat == 0xFE)
        {
            // Push end-sentinel forward before writing
            if (cat == 0xFF && i + 1 < SUBJECT_MAX_EEPROM)
            {
                writeEEPROM(subjectAddr(1, i + 1), 0xFF);
            }

            writeEEPROM(subjectAddr(2, i), subjectId);
            for (int e = 0; e < 14; e++)
            {
                writeEEPROM(subjectAddr(3, i) + e,
                            (e < (int)strlen(subject)) ? (byte)subject[e] : 0);
            }
            // Write category last (makes slot live atomically)
            writeEEPROM(subjectAddr(1, i), category);

            getSubjects(); // refresh RAM cache
            break;
        }
    }
}