#ifndef BLINK_LOGIC_H
#define BLINK_LOGIC_H

#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// "extern" means: This exists somewhere else, don't create a new one!
extern bool blinkLogic(int interval);

#endif