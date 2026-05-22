#include <Arduino.h>
#ifndef DATA_H
#define DATA_H

void writeEEPROM(int address, byte data);
byte readEEPROM(int address);

// Schedule

void addSchedule(byte hour, byte minute, byte day, byte month, byte category, byte subject, byte flags, byte interval);
void deleteSchedule();
void getSchedules();

struct Schedule
{
    byte hour;
    byte minute;
    byte day; // 1-31: once only, 0: daily, 51 - 57: weekly (1-7), 101 - 131: yearly (1-31)
    byte month;
    byte category;
    byte flags; // Bit -1: No more data behind, Bit 0: inActive, Bit 1: On Time Reminder, Bit 2: - 1 hour, Bit 3: - 24 hours
    byte subject;
};

enum Flags
{
    NO_MORE = -1,
    INACTIVE = 0,
    ON_TIME = 1,
    ONE_HOUR_BEFORE = 2,
    ONE_DAY_BEFORE = 3
};

enum Interval
{
    ONCE_ONLY,
    DAILY,
    WEEKLY,
    YEARLY
};

struct ScheduleRAM
{
    byte hour;
    byte minute;
    byte day; // 0 -> today, 1 -> tomorrow
    byte flags;
    byte category;
    byte subject;
};

extern ScheduleRAM todaySchedules[50]; // Array untuk menyimpan jadwal, maksimal 50 jadwal

// Subject

void addSubject(byte category, byte subjectId, char *subject);
void deleteSubject();
void getSubjects();

struct Subject
{
    byte category; // -1: nonActive
    byte subjectId;
    char subject[14];
};

extern Subject subjects[50]; // Array untuk menyimpan kegiatan, maksimal 50 kegiatan

// Category

enum Category
{
    HOMEWORK,
    EXAM,
    PERSONAL,
    OTHER,
};

#endif