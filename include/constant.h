#ifndef CONSTANT_H
#define CONSTANT_H

#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

extern LiquidCrystal_I2C lcd1;
extern LiquidCrystal_I2C lcd2;
extern RTC_DS3231 rtc;

extern const int btnRght;
extern const int btnLft;
extern const int btnEtr;
extern const int btnRmv;

extern const int buzzerPin;

// Free memory helper — declare here, define in main.cpp
int freeMemory();

#endif