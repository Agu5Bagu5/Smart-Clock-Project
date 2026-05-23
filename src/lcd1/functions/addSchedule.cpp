#include <Arduino.h>
#include <Wire.h>
#include "data.h"

/*  Slot layout (7 bytes):
 *   base+0 : hour
 *   base+1 : minute
 *   base+2 : day
 *   base+3 : month
 *   base+4 : category
 *   base+5 : flags   ← 0x00 = free, 0xFF = end-sentinel, else active
 *   base+6 : subject
 *
 *  This matches exactly what getSchedule.cpp reads:
 *    i*7 + SCHEDULE_BASE + 0  → hour
 *    i*7 + SCHEDULE_BASE + 1  → minute
 *    ...
 *    i*7 + SCHEDULE_BASE + 5  → flags
 *    i*7 + SCHEDULE_BASE + 6  → subject
 */
static int slotAddr(int offset, int index)
{
    return SCHEDULE_BASE + SCHEDULE_SLOT * index + offset;
}

void addSchedule(byte hour, byte minute, byte day, byte month,
                 byte category, byte subject, byte flags, byte interval)
{
    Schedule s;
    s.hour = hour;
    s.minute = minute;
    s.category = category;
    s.subject = subject;
    s.flags = flags; // caller passes ON_TIME / ONE_HOUR_BEFORE / ONE_DAY_BEFORE

    switch (interval)
    {
    case ONCE_ONLY:
        s.day = day;
        s.month = month;
        break;
    case DAILY:
        s.day = 0;
        s.month = 0;
        break;
    case WEEKLY:
        // 51-57 encode Sunday(51) … Saturday(57)
        s.day = 50 + day; // caller passes 1-7
        s.month = 0;
        break;
    case YEARLY:
        s.day = 100 + day; // 101-131
        s.month = month;
        break;
    }

    for (int i = 0; i < SCHEDULE_MAX_EEPROM; i++)
    {
        byte existingFlags = readEEPROM(slotAddr(5, i));

        if (existingFlags == FLAG_END_OF_LIST || existingFlags == FLAG_INACTIVE)
        {
            // If we're reusing an end-sentinel slot, push the sentinel forward
            if (existingFlags == FLAG_END_OF_LIST && i + 1 < SCHEDULE_MAX_EEPROM)
            {
                writeEEPROM(slotAddr(5, i + 1), FLAG_END_OF_LIST);
            }

            writeEEPROM(slotAddr(0, i), s.hour);
            writeEEPROM(slotAddr(1, i), s.minute);
            writeEEPROM(slotAddr(2, i), s.day);
            writeEEPROM(slotAddr(3, i), s.month);
            writeEEPROM(slotAddr(4, i), s.category);
            writeEEPROM(slotAddr(6, i), s.subject);
            // Write flags last so the slot is only "visible" once fully written
            writeEEPROM(slotAddr(5, i), s.flags);
            break;
        }
    }
}

void deleteSchedule(int index)
{
    if (index < 0 || index >= SCHEDULE_MAX_EEPROM)
        return;
    writeEEPROM(slotAddr(5, index), FLAG_INACTIVE);
}