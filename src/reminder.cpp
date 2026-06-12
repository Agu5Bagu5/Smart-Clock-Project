#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "reminder.h"
#include "data.h"
#include "constant.h"
#include "time-date.h"
#include "buttonPressLogic.h"
#include "symbols.h"
#include "lcd1.h"

// ─── Buzzer configurations ────────────────────────────────────────────────────
// DAY REMINDER: gentle alert, auto-stops after 6 s
static const BuzzerConfig buzzerDay = {100, 2000, 6000};
// HOUR REMINDER: medium urgency, auto-stops after 20 s
static const BuzzerConfig buzzerHour = {200, 500, 20000};
// NOW REMINDER: urgent, auto-stops after 60 s
static const BuzzerConfig buzzerNow = {500, 200, 60000};

// ─── Module state ─────────────────────────────────────────────────────────────

// Per-slot triggered flags (one struct per todaySchedules[] entry).
static ReminderFlags reminderFlags[SCHEDULE_MAX_RAM];

static int8_t activeReminderScheduleIdx = -1;
static ReminderLevel activeReminderLevel = REMINDER_DAY;
static bool reminderActive = false;

static BuzzerState buzzerState = BUZZER_IDLE;
static unsigned long buzzerStartTime = 0;

// Tracks whether lcd1 needs a full redraw.
// Set true whenever reminder state changes; cleared inside drawReminderScreen().
static bool reminderScreenDirty = false;

// Tracks the second at which we last drew the screen so we only redraw once
// per second (avoids hammering the I2C bus every loop iteration).
static int8_t lastDrawnSecond = -1;

// ─────────────────────────────────────────────────────────────────────────────

static const BuzzerConfig &getBuzzerConfig(ReminderLevel level)
{
    switch (level)
    {
    case REMINDER_HOUR:
        return buzzerHour;
    case REMINDER_NOW:
        return buzzerNow;
    default:
        return buzzerDay;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// initReminder()  — call once in setup()
// ─────────────────────────────────────────────────────────────────────────────
void initReminder()
{
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);

    resetReminderFlags();

    activeReminderScheduleIdx = -1;
    reminderActive = false;
    buzzerState = BUZZER_IDLE;
    reminderScreenDirty = false;
    lastDrawnSecond = -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// resetReminderFlags()  — call at midnight
// ─────────────────────────────────────────────────────────────────────────────
void resetReminderFlags()
{
    for (byte i = 0; i < SCHEDULE_MAX_RAM; i++)
    {
        reminderFlags[i].dayTriggered = false;
        reminderFlags[i].hourTriggered = false;
        reminderFlags[i].nowTriggered = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// checkReminders()  — call every loop iteration after lcd2Main()
//
// Trigger logic — each ScheduleRAM entry has:
//   .flags = ON_TIME | ONE_HOUR_BEFORE | ONE_DAY_BEFORE   (ReminderFlag)
//   .day   = 0 (today) | 1 (tomorrow)                     (set by getSchedules)
//
// getSchedules() already handles the advance-day decision:
//   - Entries with .day==0 are events happening TODAY at .hour:.minute
//   - Entries with .day==1 are tomorrow's events placed here so we can notify
//     today at the same stored .hour:.minute
//
// We therefore only need to compare .hour:.minute against the current wall
// clock, regardless of what .flags says about HOW far in advance to notify —
// getSchedules() has already moved the entry to today's list at the right time.
//
// The .flags value tells us only which reminder LEVEL to display (urgency /
// label), and which triggered-bit to set.
// ─────────────────────────────────────────────────────────────────────────────
void checkReminders()
{
    if (reminderActive)
        return; // one active reminder at a time

    byte nowH = nowTime.hour();
    byte nowM = nowTime.minute();

    int8_t bestIdx = -1;
    ReminderLevel bestLevel = REMINDER_DAY;

    for (byte i = 0; i < todayScheduleCount; i++)
    {
        const ScheduleRAM &s = todaySchedules[i];

        // Only consider active entries
        if (s.flags == FLAG_INACTIVE || s.flags == FLAG_END_OF_LIST)
            continue;

        // ── Determine which reminder level applies and whether it has fired ──
        ReminderLevel level;
        bool alreadyFired;

        if (s.day == 1)
        {
            // Tomorrow's event: this entry is the ONE_DAY_BEFORE notification.
            // Fire at the stored hour:minute today.
            level = REMINDER_DAY;
            alreadyFired = reminderFlags[i].dayTriggered;
        }
        else // s.day == 0 — today's event
        {
            switch (s.flags)
            {
            case ON_TIME:
                level = REMINDER_NOW;
                alreadyFired = reminderFlags[i].nowTriggered;
                break;
            case ONE_HOUR_BEFORE:
                level = REMINDER_HOUR;
                alreadyFired = reminderFlags[i].hourTriggered;
                break;
            case ONE_DAY_BEFORE:
                // ONE_DAY_BEFORE on a day==0 entry means the event is today but
                // the user chose to be notified at the stored time, which
                // getSchedules() resolved to today. Treat as ON_TIME urgency.
                level = REMINDER_NOW;
                alreadyFired = reminderFlags[i].nowTriggered;
                break;
            default:
                continue;
            }
        }

        if (alreadyFired)
            continue;

        // ── Check wall-clock match (exact minute) ────────────────────────────
        if (s.hour != nowH || s.minute != nowM)
            continue;

        // ── Keep the highest-priority hit ────────────────────────────────────
        if (bestIdx < 0 || level > bestLevel)
        {
            bestIdx = (int8_t)i;
            bestLevel = level;
        }
    }

    if (bestIdx < 0)
        return; // nothing to fire

    // ── Arm the reminder ─────────────────────────────────────────────────────
    activeReminderScheduleIdx = bestIdx;
    activeReminderLevel = bestLevel;
    reminderActive = true;
    buzzerStartTime = millis();
    buzzerState = BUZZER_BEEPING;
    reminderScreenDirty = true;
    lastDrawnSecond = -1; // force immediate redraw

    digitalWrite(buzzerPin, HIGH); // start first beep immediately

    // Mark the relevant bit so this slot doesn't fire again today
    switch (bestLevel)
    {
    case REMINDER_DAY:
        reminderFlags[bestIdx].dayTriggered = true;
        break;
    case REMINDER_HOUR:
        reminderFlags[bestIdx].hourTriggered = true;
        break;
    case REMINDER_NOW:
        reminderFlags[bestIdx].nowTriggered = true;
        break;
    }

    // Force lcd1 to clear away whatever was on screen
    lcd1.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// updateReminderBuzzer()  — call every loop iteration
// Non-blocking: uses millis() arithmetic only, no delay().
// ─────────────────────────────────────────────────────────────────────────────
void updateReminderBuzzer()
{
    if (!reminderActive)
    {
        digitalWrite(buzzerPin, LOW);
        return;
    }

    const BuzzerConfig &cfg = getBuzzerConfig(activeReminderLevel);
    unsigned long elapsed = millis() - buzzerStartTime;

    // Auto-dismiss when total timeout expires
    if (elapsed >= cfg.totalTimeout)
    {
        digitalWrite(buzzerPin, LOW);
        reminderActive = false;
        activeReminderScheduleIdx = -1;
        buzzerState = BUZZER_IDLE;
        reminderScreenDirty = true; // let drawReminderScreen clear the screen
        return;
    }

    // BUG FIX (original code): the original used a separate lastToggleTime and
    // compared it against onDuration / offDuration separately, which meant the
    // on→off transition happened after onDuration ms but the off→on transition
    // also happened after onDuration ms (not offDuration).  Using modulo
    // arithmetic on the total elapsed time is simpler and correct.
    uint16_t cycle = cfg.onDuration + cfg.offDuration;
    uint16_t position = (uint16_t)(elapsed % cycle);
    bool shouldOn = (position < cfg.onDuration);

    bool currentlyOn = (buzzerState == BUZZER_BEEPING);
    if (shouldOn != currentlyOn)
    {
        buzzerState = shouldOn ? BUZZER_BEEPING : BUZZER_SILENT;
        digitalWrite(buzzerPin, shouldOn ? HIGH : LOW);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// drawReminderScreen()  — called from lcd1Main() when a reminder is active
//
// Handles both drawing AND dismiss button polling so that everything related
// to the active reminder is in one place.  lcd1Main() just calls this and
// returns if getCurrentReminderLevel() >= 0.
//
// LCD layout:
//   Row 0:  level label (left) + event time (right)
//   Row 1:  category icon + space + subject name
// ─────────────────────────────────────────────────────────────────────────────
void drawReminderScreen()
{
    if (!reminderActive || activeReminderScheduleIdx < 0)
        return;

    // ── Dismiss on ENTER or REMOVE ───────────────────────────────────────────
    if (enterPressed() || removePressed())
    {
        dismissReminder();
        // reminderActive is now false; lcd1Main() will fall through to the
        // normal state dispatch on the very next call.
        return;
    }

    // ── Throttle redraws to once per second ──────────────────────────────────
    // The buzzer toggles at 200–500 ms; we don't need to redraw that fast.
    // Only redraw when the second ticks over or when the state just changed.
    int8_t nowSec = (int8_t)nowTime.second();
    if (!reminderScreenDirty && nowSec == lastDrawnSecond)
        return;

    reminderScreenDirty = false;
    lastDrawnSecond = nowSec;

    const ScheduleRAM &s = todaySchedules[activeReminderScheduleIdx];

    // ── Row 0: level label (cols 0-9) + time (cols 10-15) ───────────────────
    // Label
    lcd1.setCursor(0, 0);
    switch (activeReminderLevel)
    {
    case REMINDER_DAY:
        lcd1.print("Tomorrow  ");
        break;
    case REMINDER_HOUR:
        lcd1.print("In 1 Hour ");
        break;
    case REMINDER_NOW:
        lcd1.print("NOW       ");
        break;
    }

    // Event time right-aligned in cols 10-15: "HH:MM"
    char timeBuf[7];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", s.hour, s.minute);
    lcd1.setCursor(10, 0);
    lcd1.print(timeBuf);

    // ── Row 1: category icon + subject name ──────────────────────────────────
    lcd1.setCursor(0, 1);

    switch (s.category)
    {
    case HOMEWORK:
        lcd1.write(PENCIL);
        break;
    case EXAM:
        lcd1.write(WARNING);
        break;
    case PERSONAL:
        lcd1.write(PERSON);
        break;
    default:
        lcd1.write(BELL);
        break;
    }

    lcd1.print(' ');

    // BUG FIX (original reminder.cpp): the loop used getSubjectName(s) where
    // s was the RAM array index (loop counter 0..subjectCount-1).
    // getSubjectName() expects the EEPROM slot index (subjectRAMs[i].index),
    // not the subjectRAMs[] position.  Every other call site in the project
    // passes subjectRAMs[i].index — we match that here.
    const char *name = nullptr;
    for (byte i = 0; i < subjectCount; i++)
    {
        if (subjectRAMs[i].subjectId == s.subject)
        {
            name = getSubjectName(subjectRAMs[i].index); // ← correct index
            break;
        }
    }

    // Space-pad row 1 from col 2 to col 15 (14 chars)
    char row1[15];
    memset(row1, ' ', 14);
    row1[14] = '\0';

    if (name)
    {
        byte len = (byte)strlen(name);
        if (len > 14)
            len = 14;
        memcpy(row1, name, len);
    }
    else
    {
        memcpy(row1, "???", 3);
    }

    lcd1.print(row1);
}

// ─────────────────────────────────────────────────────────────────────────────
// dismissReminder()
// ─────────────────────────────────────────────────────────────────────────────
void dismissReminder()
{
    if (!reminderActive)
        return;

    digitalWrite(buzzerPin, LOW);
    reminderActive = false;
    activeReminderScheduleIdx = -1;
    buzzerState = BUZZER_IDLE;
    reminderScreenDirty = false;
    lastDrawnSecond = -1;

    lcd1.clear(); // hand lcd1 back to the normal state machine cleanly
}

// ─────────────────────────────────────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────────────────────────────────────
int8_t getCurrentReminderLevel()
{
    return (reminderActive && activeReminderScheduleIdx >= 0)
               ? (int8_t)activeReminderLevel
               : -1;
}

int8_t getCurrentReminderScheduleIndex()
{
    return activeReminderScheduleIdx;
}