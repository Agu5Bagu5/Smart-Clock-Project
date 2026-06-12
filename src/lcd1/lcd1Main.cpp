#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "symbols.h"
#include "reminder.h"

LiquidCrystal_I2C lcd1(0x27, 16, 2);

// BUG FIX: was ADD_SCHEDULE (=2), which means the first thing the user would
// see is the add-schedule wizard instead of the homepage.
State currentState = VIEW_SCHEDULE;

// ─────────────────────────────────────────────────────────────────────────────
// lcd1Main()
// Called every loop iteration.  Gives the reminder screen priority: when a
// reminder is active it draws the reminder UI and returns immediately so no
// normal state function runs and overwrites it.  Once the reminder is
// dismissed (inside drawReminderScreen / main.cpp dismiss block) normal
// operation resumes on the very next call.
// ─────────────────────────────────────────────────────────────────────────────
void lcd1Main()
{
    Serial.println("lcd1: oke");
    // ── Reminder screen takes over lcd1 when active ───────────────────────────
    // drawReminderScreen() is a no-op when no reminder is active, so this check
    // is free on the common path.
    if (getCurrentReminderLevel() >= 0)
    {
        drawReminderScreen();
        return; // suppress all normal state output while reminder is showing
    }

    // ── Normal state dispatch ─────────────────────────────────────────────────
    switch (currentState)
    {
    case HOMEPAGE:
        // homepage();
        break;
    case MENU:
        menu();
        break;
    case ADD_SCHEDULE:
        // addScheduleInterface();
        break;
    case VIEW_SCHEDULE:
        viewSchedules();
        break;
    case SUBJECTS:
        // subjects();
        break;
    default:
        break;
    }
}