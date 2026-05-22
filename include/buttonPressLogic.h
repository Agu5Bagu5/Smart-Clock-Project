#ifndef BUTTON_PRESS_LOGIC_H
#define BUTTON_PRESS_LOGIC_H

#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// "extern" means: This exists somewhere else, don't create a new one!
extern bool rightPressed();
extern bool leftPressed();
extern bool enterPressed();
extern bool removePressed();

#endif