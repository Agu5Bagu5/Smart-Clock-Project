#ifndef DATA_H
#define DATA_H

#include <Arduino.h>

// ─── EEPROM low-level ────────────────────────────────────────────────────────
void writeEEPROM(int address, byte data);
byte readEEPROM(int address);

// ─── Schedule ────────────────────────────────────────────────────────────────

/*  EEPROM layout per slot (7 bytes each, starting at base address 860):
 *   offset 0 : hour
 *   offset 1 : minute
 *   offset 2 : day    (1-31 = once, 0 = daily, 51-57 = weekly, 101-131 = yearly)
 *   offset 3 : month
 *   offset 4 : category
 *   offset 5 : flags  (0 = inactive / slot free, 1 = ON_TIME, 2 = ONE_HOUR, 3 = ONE_DAY, 0xFF = end-of-list sentinel)
 *   offset 6 : subject id
 */
#define SCHEDULE_BASE 860
#define SCHEDULE_SLOT 7
#define SCHEDULE_MAX_EEPROM 100 // reduced from 200 to save iteration time
#define SCHEDULE_MAX_RAM 20     // reduced from 50 — Uno has only 2 KB SRAM
#include "time-date.h"          // for decltype(nowTime) in getSchedules()

void addSchedule(byte hour, byte minute, byte day, byte month,
                 byte category, byte subject, byte flags, byte interval);

void cleanupExpiredSchedules(const DateTime &today = nowTime);

void deleteSchedule(int index);

void getSchedules(const decltype(nowTime) &referenceDate = nowTime);

struct Schedule
{
    byte hour;
    byte minute;
    byte day;
    byte month;
    byte category;
    byte flags;
    byte subject;
};

// flags values
#define FLAG_END_OF_LIST 0xFF // no more data beyond this slot
#define FLAG_INACTIVE 0x00    // slot is free / deleted

enum Interval : byte
{
    ONCE_ONLY = 0,
    DAILY = 1,
    WEEKLY = 2,
    YEARLY = 3
};

// reminder flag values stored in flags field when active
enum ReminderFlag : byte
{
    ON_TIME = 1,
    ONE_HOUR_BEFORE = 2,
    ONE_DAY_BEFORE = 3
};

struct ScheduleRAM
{
    byte hour;
    byte minute;
    byte day; // 0 = today, 1 = tomorrow
    byte flags;
    byte category;
    byte subject;
    byte eepromSlot;
};

extern ScheduleRAM todaySchedules[SCHEDULE_MAX_RAM];
extern byte todayScheduleCount; // how many valid entries are in todaySchedules

// ─── Subject ─────────────────────────────────────────────────────────────────

/*  EEPROM layout per slot (16 bytes each, starting at address 0):
 *   offset 0  : (unused / alignment)
 *   offset 1  : category  (0xFF = end-of-list sentinel, 0xFE = deleted)
 *   offset 2  : subjectId
 *   offset 3…16 : subject name (14 chars, null-terminated)
 */
#define SUBJECT_BASE 0
#define SUBJECT_SLOT 16
#define SUBJECT_MAX_EEPROM 50
#define SUBJECT_MAX_RAM 50 // reduced from 50

void addSubject(byte category, byte subjectId, const char *subject);
void getSubjects();
char *getSubjectName(byte index);

struct Subject
{
    byte category;
    byte subjectId;
    char subject[14];
};

struct SubjectRAM
{
    byte category;
    byte subjectId;
    byte index;
};

extern byte subjectCount; // how many valid entries are in subjects[]
extern SubjectRAM subjectRAMs[SUBJECT_MAX_RAM];

// ─── Category ────────────────────────────────────────────────────────────────
enum Category : byte
{
    HOMEWORK = 0,
    EXAM = 1,
    PERSONAL = 2,
    OTHER = 3
};

#endif // DATA_H