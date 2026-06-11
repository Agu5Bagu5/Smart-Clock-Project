#ifndef BUTTON_PRESS_LOGIC_H
#define BUTTON_PRESS_LOGIC_H

#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// "extern" means: This exists somewhere else, don't create a new one!
extern bool rightPressed(unsigned long interval = 0);
extern bool leftPressed(unsigned long interval = 0);
extern bool enterPressed(unsigned long interval = 0);
extern bool removePressed(unsigned long interval = 0);

#endif