#ifndef TIME_DATE_H
#define TIME_DATE_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

// "extern" means: This exists somewhere else, don't create a new one!
extern DateTime nowTime;      // Variable untuk menyimpan waktu saat ini
extern DateTime tomorrowTime; // Variable untuk menyimpan waktu besok

#endif