#ifndef REMINDER_H
#define REMINDER_H

#include <Arduino.h>
#include "data.h"

// ─── Reminder Levels ──────────────────────────────────────────────────────────
enum ReminderLevel : byte
{
    REMINDER_DAY = 0,
    REMINDER_HOUR = 1,
    REMINDER_NOW = 2
};

// ─── Buzzer States ────────────────────────────────────────────────────────────
enum BuzzerState : byte
{
    BUZZER_IDLE = 0,
    BUZZER_BEEPING = 1,
    BUZZER_SILENT = 2
};

// ─── Reminder Trigger Flags ───────────────────────────────────────────────────
// Per-schedule tracking to prevent repeated triggering
struct ReminderFlags
{
    bool dayTriggered;  // ONE_DAY_BEFORE reminder triggered
    bool hourTriggered; // ONE_HOUR_BEFORE reminder triggered
    bool nowTriggered;  // AT_TIME reminder triggered
};

// ─── Buzzer Configuration ─────────────────────────────────────────────────────
struct BuzzerConfig
{
    uint16_t onDuration;   // Beep length (ms)
    uint16_t offDuration;  // Silence length (ms)
    uint16_t totalTimeout; // Total reminder duration (ms)
};

// ─── Public Functions ─────────────────────────────────────────────────────────

// Initialize reminder system (call once in setup())
void initReminder();

// Check for triggered reminders and update state (call each loop iteration)
void checkReminders();

// Update buzzer timing based on current reminder (call each loop iteration)
void updateReminderBuzzer();

// Draw reminder screen on LCD1 (managed internally, but can be called from lcd1Main)
void drawReminderScreen();

// Dismiss current reminder and return to idle state
void dismissReminder();

// Reset all trigger flags (call at midnight)
void resetReminderFlags();

// Get current active reminder level (-1 if none)
int8_t getCurrentReminderLevel();

// Get current active reminder schedule index (-1 if none)
int8_t getCurrentReminderScheduleIndex();

#endif // REMINDER_H
