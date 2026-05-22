#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include "data.h"
#include "time-date.h"

static int adress(int e, int index)
{
    return ((860 + e) + 7 * index);
}

void addSchedule(byte hour, byte minute, byte day, byte month, byte category, byte subject, byte flags, byte interval)
{
    Schedule newSchedule;

    if (interval == ONCE_ONLY)
    {
        newSchedule.day = day;
        newSchedule.month = month;
    }
    else if (interval == DAILY)
    {
        newSchedule.day = 0; // 0 menandakan jadwal ini untuk setiap hari
        newSchedule.month = 0;
    }
    else if (interval == WEEKLY)
    {
        newSchedule.day = 50 + 1 + day; // 51-57 menandakan jadwal ini untuk setiap minggu, dengan 51 untuk hari Minggu, 52 untuk hari Senin, dan seterusnya
        newSchedule.month = 0;
    }
    else if (interval == YEARLY)
    {
        newSchedule.day = 100 + day; // 101-131 menandakan jadwal ini untuk setiap tahun, dengan day untuk menandakan tanggalnya
        newSchedule.month = month;
    }

    newSchedule.hour = hour;
    newSchedule.minute = minute;
    newSchedule.category = category;
    newSchedule.subject = subject;
    newSchedule.flags = flags;

    for (int i = 0; i < 200; i++)
    {
        byte flag = readEEPROM((860 + 6) + 7 * i);
        if (flag == -1 || flag == 0)
        {
            if (flag == -1)
            {
                writeEEPROM(adress(6, i) + 7, -1);
            }

            writeEEPROM(adress(6, i), newSchedule.flags);
            writeEEPROM(adress(1, i), newSchedule.hour);
            writeEEPROM(adress(2, i), newSchedule.minute);
            writeEEPROM(adress(3, i), newSchedule.day);
            writeEEPROM(adress(4, i), newSchedule.month);
            if (readEEPROM(adress(5, i)) != newSchedule.category)
            {
                writeEEPROM(adress(5, i), newSchedule.category);
            }
            if (readEEPROM(adress(7, i)) != newSchedule.subject)
            {
                writeEEPROM(adress(7, i), newSchedule.subject);
            }
            break;
        }
    }
}