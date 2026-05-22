#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "data.h"
#include "RTClib.h"
// #include "time-date.h"

ScheduleRAM displayedSchedules[50];

void viewSchedules()
{
    // Logika untuk menampilkan homepage
    lcd1.setCursor(0, 0);
    lcd1.print(byte(displayedSchedules[0].category));
    lcd1.print(" " + displayedSchedules[0].category);
    lcd1.setCursor(0, 1);
    lcd1.print(byte(displayedSchedules[1].category));
    lcd1.print("Test Schedule");
}

// char *getSubject()
// {
//     for (int i; i < 50; i++)
//     {
//     }
// }

void getDisplayedSchedules(DateTime dateDisplayed)
{
    // Logika untuk mengambil jadwal dari EEPROM dan menampilkannya
    int todayIndex = 0; // Indeks untuk menyimpan jadwal hari ini
    for (int i = 0; i < 200; i++)
    {
        // Contoh membaca jadwal dari EEPROM
        byte flags = readEEPROM(i * 7 + 863 + 5); // Alamat untuk flags
        if (flags != 0)                           // Cek apakah slot jadwal ini terisi
        {
            byte scheduleDay = readEEPROM(i * 7 + 863 + 2);
            bool isOnce = true;
            if (scheduleDay > 100 && scheduleDay < 132) // Jika day 101-131, berarti jadwal ini untuk setiap tahun, jadi kita set day ke hari ini untuk memudahkan pengecekan
            {
                scheduleDay = scheduleDay - 100;
                isOnce = false;
            }
            byte scheduleMonth = readEEPROM(i * 7 + 863 + 3);
            if ((scheduleDay == dateDisplayed.day() && scheduleMonth == dateDisplayed.month()) || scheduleDay == 0 || (scheduleDay > 50 && scheduleDay < 58 && scheduleMonth - 50 == dateDisplayed.dayOfTheWeek())) // Cek apakah jadwal ini untuk hari ini atau setiap hari
            {
                // Jika jadwal sesuai dengan hari dan bulan yang diminta, tampilkan atau simpan untuk ditampilkan
                displayedSchedules[todayIndex].hour = readEEPROM(i * 7 + 863 + 0);
                displayedSchedules[todayIndex].minute = readEEPROM(i * 7 + 863 + 1);
                displayedSchedules[todayIndex].day = 0;
                displayedSchedules[todayIndex].category = readEEPROM(i * 7 + 863 + 4);
                displayedSchedules[todayIndex].subject = readEEPROM(i * 7 + 863 + 6); // Alamat untuk subject
                todayIndex++;
                // Tampilkan jadwal atau simpan dalam array untuk ditampilkan nanti

                if (scheduleDay > 0 && scheduleDay < 32 && isOnce)
                {
                    writeEEPROM(i * 7 + 863 + 5, 0); // Set day ke 101-131 untuk menandakan bahwa jadwal ini sudah diproses untuk hari ini, sehingga tidak akan diproses lagi sampai besok
                }
            }
        }
        else if (flags == -1)
        {
            // Jika flags -1, berarti tidak ada jadwal lagi setelah ini, jadi kita bisa berhenti mencari
            break;
        }
    }
}