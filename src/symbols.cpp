#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "constant.h"
#include "symbols.h"

// All icon arrays must be in RAM (not PROGMEM) because createChar() needs a
// pointer it can read with ordinary pointer arithmetic on AVR.

static byte iconHomework[8] = {
    B00010, B00110, B01110, B11100,
    B11000, B10000, B00000, B00000};

static byte iconExam[8] = {
    B00100, B00100, B00100, B00100,
    B00100, B00000, B00100, B00000};

static byte iconPersonal[8] = {
    B00100, B01110, B00100, B00100,
    B01110, B10001, B00000, B00000};

static byte iconBell[8] = {
    B00100, B01110, B01110, B01110,
    B11111, B00100, B00000, B00100};

static byte iconOther[8] = {
    B00100, B10101, B01110, B11111,
    B01110, B10101, B00100, B00000};

static byte iconClock[8] = {
    B01110, B10001, B10101, B10101,
    B10001, B01110, B00000, B00000};

static byte iconCalendar[8] = {
    B11111, B10001, B11111, B10101,
    B10001, B10001, B11111, B00000};

void initSymbols()
{
    lcd1.createChar(PENCIL, iconHomework);
    lcd1.createChar(WARNING, iconExam);
    lcd1.createChar(PERSON, iconPersonal);
    lcd1.createChar(BELL, iconBell);
    lcd1.createChar(STAR, iconOther);
    lcd1.createChar(TIME_ICO, iconClock);
    lcd1.createChar(CALENDAR, iconCalendar);
}