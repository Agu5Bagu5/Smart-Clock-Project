#include <Arduino.h>
#include <data.h>
#include "time-date.h"

void getSchedules()
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
            if ((scheduleDay == nowTime.day() && scheduleMonth == nowTime.month()) || scheduleDay == 0 || (scheduleDay > 50 && scheduleDay < 58 && scheduleMonth - 50 == nowTime.dayOfTheWeek())) // Cek apakah jadwal ini untuk hari ini atau setiap hari
            {
                // Jika jadwal sesuai dengan hari dan bulan yang diminta, tampilkan atau simpan untuk ditampilkan
                todaySchedules[todayIndex].hour = readEEPROM(i * 7 + 863 + 0);
                todaySchedules[todayIndex].minute = readEEPROM(i * 7 + 863 + 1);
                todaySchedules[todayIndex].day = 0;
                todaySchedules[todayIndex].category = readEEPROM(i * 7 + 863 + 4);
                todaySchedules[todayIndex].subject = readEEPROM(i * 7 + 863 + 6); // Alamat untuk subject
                todayIndex++;
                // Tampilkan jadwal atau simpan dalam array untuk ditampilkan nanti

                if (scheduleDay > 0 && scheduleDay < 32 && isOnce)
                {
                    writeEEPROM(i * 7 + 863 + 5, 0); // Set day ke 101-131 untuk menandakan bahwa jadwal ini sudah diproses untuk hari ini, sehingga tidak akan diproses lagi sampai besok
                }
            }
            else if ((scheduleDay == tomorrowTime.day() && scheduleMonth == tomorrowTime.month()) || (scheduleDay > 50 && scheduleDay < 58 && scheduleMonth - 50 == tomorrowTime.dayOfTheWeek()))
            {
                if (flags == 3) // Cek apakah kategori jadwal ini adalah "- 1 day reminder"
                {
                    // Jika jadwal besok adalah kategori "Penting", tampilkan atau simpan untuk ditampilkan
                    todaySchedules[todayIndex].hour = readEEPROM(i * 7 + 863 + 0);
                    todaySchedules[todayIndex].minute = readEEPROM(i * 7 + 863 + 1);
                    todaySchedules[todayIndex].day = 1;
                    todaySchedules[todayIndex].category = readEEPROM(i * 7 + 863 + 4);
                    todaySchedules[todayIndex].subject = readEEPROM(i * 7 + 863 + 6); // Alamat untuk subject
                    todayIndex++;
                    // Tampilkan jadwal atau simpan dalam array untuk ditampilkan nanti
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