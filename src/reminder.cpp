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

// ─── Buzzer Configurations ────────────────────────────────────────────────────
// DAY REMINDER: Lowest priority, auto-stop after 6 seconds
static const BuzzerConfig buzzerDay = {
    100,  // 100ms beep
    2000, // 2000ms silence
    6000  // 6 seconds total
};

// HOUR REMINDER: Medium priority, auto-stop after 20 seconds, user dismissible
static const BuzzerConfig buzzerHour = {
    200,  // 200ms beep
    500,  // 500ms silence
    20000 // 20 seconds total
};

// NOW REMINDER: Highest priority, auto-stop after 60 seconds, user dismissible
static const BuzzerConfig buzzerNow = {
    500,  // 500ms beep
    200,  // 200ms silence
    60000 // 60 seconds total
};

// ─── Reminder State Variables ─────────────────────────────────────────────────

// Trigger flags per schedule
static ReminderFlags reminderFlags[SCHEDULE_MAX_RAM];

// Current active reminder
static int8_t activeReminderScheduleIdx = -1;
static ReminderLevel activeReminderLevel = REMINDER_DAY;
static bool reminderActive = false;

// Buzzer state
static BuzzerState buzzerState = BUZZER_IDLE;
static unsigned long buzzerStartTime = 0;
static unsigned long buzzerLastToggleTime = 0;

// LCD1 display state
static bool reminderScreenDirty = false;

// ─── Helper: Get buzzer config for reminder level ────────────────────────────
static const BuzzerConfig &getBuzzerConfig(ReminderLevel level)
{
    switch (level)
    {
    case REMINDER_DAY:
        return buzzerDay;
    case REMINDER_HOUR:
        return buzzerHour;
    case REMINDER_NOW:
        return buzzerNow;
    default:
        return buzzerDay;
    }
}

// ─── Helper: Calculate time difference in minutes ─────────────────────────────
// Returns minutes until/since the given time
static int16_t minutesUntilTime(byte scheduleHour, byte scheduleMinute,
                                const DateTime &now)
{
    uint16_t scheduleTimeInMinutes = (uint16_t)scheduleHour * 60 + scheduleMinute;
    uint16_t currentTimeInMinutes = (uint16_t)now.hour() * 60 + now.minute();

    int16_t diff = scheduleTimeInMinutes - currentTimeInMinutes;
    return diff;
}

// ─── Initialize Reminder System ────────────────────────────────────────────────
void initReminder()
{
    // Set buzzer pin as OUTPUT
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);

    // Initialize trigger flags to all false
    resetReminderFlags();

    activeReminderScheduleIdx = -1;
    reminderActive = false;
    buzzerState = BUZZER_IDLE;
    reminderScreenDirty = true;
}

// ─── Reset Reminder Trigger Flags ─────────────────────────────────────────────
// Called at midnight to allow all reminders to trigger again
void resetReminderFlags()
{
    for (byte i = 0; i < SCHEDULE_MAX_RAM; i++)
    {
        reminderFlags[i].dayTriggered = false;
        reminderFlags[i].hourTriggered = false;
        reminderFlags[i].nowTriggered = false;
    }
}

// ─── Check for Triggered Reminders ────────────────────────────────────────────
void checkReminders()
{
    // If already active, don't check for new ones (user must dismiss or auto-timeout)
    if (reminderActive && activeReminderScheduleIdx >= 0)
    {
        return;
    }

    // Scan all today's schedules for reminders to trigger
    int8_t highestPriorityIdx = -1;
    ReminderLevel highestPriority = REMINDER_DAY;

    for (byte i = 0; i < todayScheduleCount; i++)
    {
        byte flags = todaySchedules[i].flags;

        // Skip inactive schedules
        if (flags == FLAG_INACTIVE || flags == FLAG_END_OF_LIST)
            continue;

        byte hour = todaySchedules[i].hour;
        byte minute = todaySchedules[i].minute;
        byte day = todaySchedules[i].day;

        int16_t minutesDiff;
        ReminderLevel checkLevel = REMINDER_DAY;
        bool shouldTrigger = false;

        // Calculate time difference based on schedule day (today=0, tomorrow=1)
        if (day == 0)
        {
            // Today's schedule
            minutesDiff = minutesUntilTime(hour, minute, nowTime);
        }
        else if (day == 1)
        {
            // Tomorrow's schedule - calculate diff from tomorrow's perspective
            minutesDiff = minutesUntilTime(hour, minute, tomorrowTime);
        }
        else
        {
            continue; // Invalid day value
        }

        // Check for reminders based on reminder flag
        switch (flags)
        {
        case ON_TIME:
            // Trigger at exact schedule time
            if (minutesDiff >= -1 && minutesDiff <= 0)
            {
                if (!reminderFlags[i].nowTriggered)
                {
                    checkLevel = REMINDER_NOW;
                    shouldTrigger = true;
                }
            }
            break;

        case ONE_HOUR_BEFORE:
            // Trigger 1 hour before (59-61 minutes)
            if (minutesDiff >= 59 && minutesDiff <= 61)
            {
                if (!reminderFlags[i].hourTriggered)
                {
                    checkLevel = REMINDER_HOUR;
                    shouldTrigger = true;
                }
            }
            // Also trigger at exact time
            else if (minutesDiff >= -1 && minutesDiff <= 0)
            {
                if (!reminderFlags[i].nowTriggered)
                {
                    checkLevel = REMINDER_NOW;
                    shouldTrigger = true;
                }
            }
            break;

        case ONE_DAY_BEFORE:
            // Trigger 1 day before (1439-1441 minutes, accounting for exact day transition)
            // Simplified: if it's tomorrow's schedule, it's the "day before" reminder
            if (day == 1 && minutesDiff >= 1380 && minutesDiff <= 1500)
            {
                if (!reminderFlags[i].dayTriggered)
                {
                    checkLevel = REMINDER_DAY;
                    shouldTrigger = true;
                }
            }
            // Trigger 1 hour before (59-61 minutes of the same day)
            else if (day == 0 && minutesDiff >= 59 && minutesDiff <= 61)
            {
                if (!reminderFlags[i].hourTriggered)
                {
                    checkLevel = REMINDER_HOUR;
                    shouldTrigger = true;
                }
            }
            // Trigger at exact time
            else if (day == 0 && minutesDiff >= -1 && minutesDiff <= 0)
            {
                if (!reminderFlags[i].nowTriggered)
                {
                    checkLevel = REMINDER_NOW;
                    shouldTrigger = true;
                }
            }
            break;

        default:
            break;
        }

        // Track reminder with highest priority
        if (shouldTrigger && checkLevel > highestPriority)
        {
            highestPriority = checkLevel;
            highestPriorityIdx = i;
        }
    }

    // If we found a reminder to trigger, activate it
    if (highestPriorityIdx >= 0)
    {
        activeReminderScheduleIdx = highestPriorityIdx;
        activeReminderLevel = highestPriority;
        reminderActive = true;
        buzzerStartTime = millis();
        buzzerState = BUZZER_BEEPING;
        buzzerLastToggleTime = millis();
        reminderScreenDirty = true;

        // Mark this reminder level as triggered
        switch (highestPriority)
        {
        case REMINDER_DAY:
            reminderFlags[highestPriorityIdx].dayTriggered = true;
            break;
        case REMINDER_HOUR:
            reminderFlags[highestPriorityIdx].hourTriggered = true;
            break;
        case REMINDER_NOW:
            reminderFlags[highestPriorityIdx].nowTriggered = true;
            break;
        }
    }
}

// ─── Update Buzzer Timing ─────────────────────────────────────────────────────
// Manages beep/silence pattern and auto-timeout using millis()
void updateReminderBuzzer()
{
    if (!reminderActive || activeReminderScheduleIdx < 0)
    {
        // No active reminder, keep buzzer off
        digitalWrite(buzzerPin, LOW);
        return;
    }

    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - buzzerStartTime;
    const BuzzerConfig &config = getBuzzerConfig(activeReminderLevel);

    // Check if total timeout reached - auto-stop
    if (elapsedTime >= config.totalTimeout)
    {
        digitalWrite(buzzerPin, LOW);
        reminderActive = false;
        activeReminderScheduleIdx = -1;
        buzzerState = BUZZER_IDLE;
        reminderScreenDirty = true;
        return;
    }

    // Calculate position in beep/silence cycle
    uint16_t cycleLength = config.onDuration + config.offDuration;
    uint16_t positionInCycle = elapsedTime % cycleLength;

    // Determine if we should be beeping
    bool shouldBeep = (positionInCycle < config.onDuration);

    // Update buzzer output if state changed
    if (shouldBeep != (buzzerState == BUZZER_BEEPING))
    {
        if (shouldBeep)
        {
            digitalWrite(buzzerPin, HIGH);
            buzzerState = BUZZER_BEEPING;
        }
        else
        {
            digitalWrite(buzzerPin, LOW);
            buzzerState = BUZZER_SILENT;
        }
    }
}

// ─── Draw Reminder Screen on LCD1 ─────────────────────────────────────────────
void drawReminderScreen()
{
    if (!reminderActive || activeReminderScheduleIdx < 0)
    {
        return;
    }

    // Only update if state changed (to avoid flickering)
    if (!reminderScreenDirty)
    {
        return;
    }
    reminderScreenDirty = false;

    int8_t idx = activeReminderScheduleIdx;
    byte subjectId = todaySchedules[idx].subject;

    // Get subject name from subjectRAMs[]
    char subjectName[14];
    strcpy_P(subjectName, PSTR("Unknown"));

    for (byte s = 0; s < subjectCount; s++)
    {
        if (subjectRAMs[s].subjectId == subjectId)
        {
            // Get full subject name from EEPROM via getSubjectName()
            const char *name = getSubjectName(s);
            if (name != nullptr)
            {
                strncpy(subjectName, name, sizeof(subjectName) - 1);
                subjectName[sizeof(subjectName) - 1] = '\0';
            }
            break;
        }
    }

    // Clear LCD1 and draw reminder screen
    lcd1.clear();

    // Line 1: Reminder type
    switch (activeReminderLevel)
    {
    case REMINDER_DAY:
        lcd1.setCursor(1, 0);
        lcd1.print(F("Tomorrow Event"));
        break;
    case REMINDER_HOUR:
        lcd1.setCursor(2, 0);
        lcd1.print(F("In 1 Hour"));
        break;
    case REMINDER_NOW:
        lcd1.setCursor(6, 0);
        lcd1.print(F("NOW"));
        break;
    }

    // Line 2: Category icon + Subject name
    lcd1.setCursor(0, 1);

    // Print category icon (using symbols)
    byte category = todaySchedules[idx].category;
    switch (category)
    {
    case HOMEWORK:
        lcd1.write(PENCIL); // Pencil icon for homework
        break;
    case EXAM:
        lcd1.write(WARNING); // Warning icon for exam
        break;
    case PERSONAL:
        lcd1.write(PERSON); // Person icon for personal
        break;
    case OTHER:
        lcd1.write(BELL); // Bell icon for other
        break;
    default:
        lcd1.write(BELL);
        break;
    }

    // Print subject name (pad to 14 chars)
    lcd1.print(" ");
    lcd1.print(subjectName);
}

// ─── Dismiss Current Reminder ─────────────────────────────────────────────────
void dismissReminder()
{
    if (!reminderActive)
    {
        return;
    }

    digitalWrite(buzzerPin, LOW);
    reminderActive = false;
    activeReminderScheduleIdx = -1;
    buzzerState = BUZZER_IDLE;
    reminderScreenDirty = true;
}

// ─── Get Current Active Reminder Level ────────────────────────────────────────
int8_t getCurrentReminderLevel()
{
    if (!reminderActive || activeReminderScheduleIdx < 0)
    {
        return -1;
    }
    return (int8_t)activeReminderLevel;
}

// ─── Get Current Active Schedule Index ────────────────────────────────────────
int8_t getCurrentReminderScheduleIndex()
{
    return activeReminderScheduleIdx;
}
