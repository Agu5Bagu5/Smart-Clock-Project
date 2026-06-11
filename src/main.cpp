#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include "lcd1.h"
#include "lcd2.h"
#include "constant.h"
#include "symbols.h"
#include "data.h"
#include "reminder.h"
#include "buttonPressLogic.h"

// ─── Button pins ─────────────────────────────────────────────────────────────
// NOTE: subjects[], subjectCount, todaySchedules[], todayScheduleCount
// are defined in getSubjects.cpp / getSchedule.cpp respectively.
// They are extern-declared in data.h — do NOT redefine them here.
const int btnRght = 8;
const int btnLft = 9;
const int btnEtr = 10;
const int btnRmv = 11;

RTC_DS3231 rtc;

// ─── Buzzer pin ───────────────────────────────────────────────────────────────
const int buzzerPin = 12;

// ─── Free-memory helper (AVR-specific) ───────────────────────────────────────
extern int __heap_start;
extern int *__brkval;

int freeMemory()
{
  int v;
  return (int)&v - (__brkval == nullptr
                        ? (int)&__heap_start
                        : (int)__brkval);
}

// ─────────────────────────────────────────────────────────────────────────────
void setup()
{
  Serial.begin(9600);

  Wire.begin();

  // LCD init
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();

  // Button pins
  pinMode(btnRght, INPUT);
  pinMode(btnLft, INPUT);
  pinMode(btnEtr, INPUT);
  pinMode(btnRmv, INPUT);

  // Custom symbols (must happen after lcd1.init())
  initSymbols();

  // RTC
  if (!rtc.begin())
  {
    lcd1.setCursor(0, 0);
    lcd1.print("RTC not found!");
    while (1)
      ;
  }
  if (rtc.lostPower())
  {
    // Set to compile time if clock lost power
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Load cached data from EEPROM
  getSubjects();

  cleanupExpiredSchedules();
  getSchedules();

  // Initialize reminder system
  initReminder();

  // Debug: print free RAM over Serial so you can spot memory issues
  Serial.print(F("Free RAM after setup: "));
  Serial.println(freeMemory());
}

void loop()
{
  // Check for new reminders (highest priority, runs before LCD1 states)
  checkReminders();

  // Update buzzer timing (non-blocking, millis-based)
  updateReminderBuzzer();

  // Handle button dismiss during reminder
  if (getCurrentReminderLevel() >= 0)
  {
    if (enterPressed() || removePressed())
    {
      dismissReminder();
    }
  }

  // Update clock display (runs every loop, continues during reminders)
  clockDisplay();

  // Update LCD1 with priority to reminders
  lcd1Main();
}