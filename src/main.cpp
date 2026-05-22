#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include "lcd1.h"
#include "lcd2.h"
#include "constant.h"
#include "symbols.h"
#include "data.h"

Subject subjects[50];
ScheduleRAM todaySchedules[50];

const int btnRght = 9;
const int btnLft = 8;
const int btnEtr = 10;
const int btnRmv = 11;

void setup()
{
  // lcd config
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();

  // button pinMode
  pinMode(btnRght, INPUT);
  pinMode(btnLft, INPUT);
  pinMode(btnEtr, INPUT);
  pinMode(btnRmv, INPUT);

  initSymbols();

  // typing config
  lcd1.setCursor(0, 0);
  lcd1.print("Txt|");
  lcd1.setCursor(0, 1);
  lcd1.print("Sel|");

  // RTC config
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  if (rtc.lostPower())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop()
{
  lcd1Main();
  lcd2Main();
}
