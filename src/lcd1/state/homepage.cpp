#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"

void homepage()
{
    // Logika untuk menampilkan homepage
    lcd1.setCursor(0, 0);
    lcd1.print("Next Schedule:");
    lcd1.setCursor(0, 1);
    lcd1.print("Test Schedule");
}