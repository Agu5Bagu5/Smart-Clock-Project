#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "symbols.h"

// ===== CATEGORY ICONS =====

// 0 = HOMEWORK / PENCIL
byte iconHomework[8] = {
    B00010,
    B00110,
    B01110,
    B11100,
    B11000,
    B10000,
    B00000,
    B00000};

// 1 = EXAM / WARNING
byte iconExam[8] = {
    B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B00000,
    B00100,
    B00000};

// 2 = PERSONAL / PERSON
byte iconPersonal[8] = {
    B00100,
    B01110,
    B00100,
    B00100,
    B01110,
    B10001,
    B00000,
    B00000};

// 3 = REMINDER / BELL
byte iconBell[8] = {
    B00100,
    B01110,
    B01110,
    B01110,
    B11111,
    B00100,
    B00000,
    B00100};

// 4 = OTHER / STAR
byte iconOther[8] = {
  B00100,
  B10101,
  B01110,
  B11111,
  B01110,
  B10101,
  B00100,
  B00000
};

// 5 = CLOCK / TIME
byte iconClock[8] = {
    B01110,
    B10001,
    B10101,
    B10101,
    B10001,
    B01110,
    B00000,
    B00000};

// 6 = DATE / CALENDAR
byte iconCalendar[8] = {
    B11111,
    B10001,
    B11111,
    B10101,
    B10001,
    B10001,
    B11111,
    B00000};

void initSymbols()
{
lcd1.createChar(PENCIL, iconHomework);
lcd1.createChar(WARNING, iconExam);
lcd1.createChar(PERSON, iconPersonal);
lcd1.createChar(BELL, iconBell);
lcd1.createChar(STAR, iconOther);
lcd1.createChar(TIME, iconClock);
lcd1.createChar(CALENDER, iconCalendar);
}