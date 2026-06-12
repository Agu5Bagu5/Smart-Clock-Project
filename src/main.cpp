#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include "lcd1.h"
#include "lcd2.h"
#include "constant.h"
#include "symbols.h"
#include "data.h"
#include "reminder.h"

// ─── Button pins ─────────────────────────────────────────────────────────────
const int btnRght = 3;
const int btnLft = 10;
const int btnEtr = 6;
const int btnRmv = 12;

// RTC is used by lcd2Main/clockDisplay and extern-declared in constant.h
RTC_DS3231 rtc;

// Buzzer on pin 12 — first free digital pin after the four button inputs.
// Wire piezo/buzzer between pin 12 and GND.
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

  // // LCD init — must happen before initSymbols() which calls createChar()
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();

  // Serial.println("Setup started");

  // Button pins
  pinMode(btnRght, INPUT);
  pinMode(btnLft, INPUT);
  pinMode(btnEtr, INPUT);
  pinMode(btnRmv, INPUT);

  // Custom character bitmaps loaded into lcd1 CGRAM
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
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Load schedule and subject caches from external EEPROM
  getSubjects();
  cleanupExpiredSchedules();
  getSchedules();

  // Initialise reminder subsystem (sets buzzerPin OUTPUT, clears flags)
  initReminder();

  Serial.print(F("Free RAM after setup: "));
  Serial.println(freeMemory());
}

void loop()
{
  Serial.println("overall: oke");
  // ── STEP 1: refresh nowTime from RTC ─────────────────────────────────────
  // BUG FIX: lcd2Main() / clockDisplay() MUST run first every iteration.
  // It is the only place that calls nowTime = rtc.now().
  // checkReminders() reads nowTime to decide whether to fire — if lcd2Main()
  // ran after it, the first loop iteration would check against an uninitialised
  // DateTime (the RTClib epoch, ~2000-01-01 00:00:00), potentially triggering
  // every schedule at once on startup.
  lcd2Main();

  // ── STEP 2: check / drive the reminder subsystem ─────────────────────────
  // nowTime is guaranteed fresh from Step 1.
  checkReminders();       // scan todaySchedules[], arm if due
  updateReminderBuzzer(); // non-blocking beep pattern via millis()

  // ── STEP 3: update lcd1 ───────────────────────────────────────────────────
  // lcd1Main() handles its own reminder priority: when a reminder is active
  // it calls drawReminderScreen() (which also handles dismiss buttons) and
  // returns without running any normal state function.
  lcd1Main();
}