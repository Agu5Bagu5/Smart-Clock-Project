#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "buttonPressLogic.h"
#include "blinkLogic.h"

// ─── State ────────────────────────────────────────────────────────────────────
static int displayShift = 0;
static int activeRow = 0; // 0-7 = alphabet index in window, -1 = space, -2 = OK
static bool dipActive = true;

static const char alphabets[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int ALPHA_LEN = 26;
static const int WINDOW_SIZE = 8;
static char displayedChars[WINDOW_SIZE];

static char typedText[14];
static bool confirmed;

// ─── Helpers ─────────────────────────────────────────────────────────────────
static void updateDisplayedChars()
{
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        int idx = i + displayShift;
        displayedChars[i] = (idx < ALPHA_LEN) ? alphabets[idx] : ' ';
    }
}

static void renderRow1()
{
    // Columns 0-2: context buttons
    if (activeRow == -2)
    {
        lcd1.setCursor(0, 1);
        lcd1.print("Ok ");
    }
    else
    {
        lcd1.setCursor(0, 1);
        lcd1.print("   ");
    }

    // Column 3: separator
    lcd1.setCursor(3, 1);
    lcd1.print("Sp|"); // "Sp" = space key

    // Columns 6-13: letter window (show underscore cursor on active)
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        lcd1.setCursor(6 + i, 1);
        if (i == activeRow && !dipActive)
        {
            lcd1.print('_');
        }
        else
        {
            lcd1.print(displayedChars[i]);
        }
    }
}

// ─── Public reset ─────────────────────────────────────────────────────────────
void resetTyping()
{
    displayShift = 0;
    activeRow = 0;
    dipActive = true;
    confirmed = false;
    memset(typedText, 0, sizeof(typedText));
    updateDisplayedChars();
}

// ─── Main typing function ─────────────────────────────────────────────────────
// Returns "" while the user is still typing.
// Returns the typed string once the user presses OK (activeRow == -2 + enter).
// Caller must call resetTyping() when done consuming the result.
const char *typing()
{
    if (confirmed)
        return typedText; // already done; caller hasn't reset yet

    // ── RIGHT ────────────────────────────────────────────────────────────────
    if (rightPressed())
    {
        if (activeRow >= WINDOW_SIZE - 1)
        {
            // Scroll window right if possible
            if (displayShift + WINDOW_SIZE < ALPHA_LEN)
            {
                displayShift++;
            }
            else
            {
                // Wrap: jump to OK
                activeRow = -2;
            }
        }
        else if (activeRow == -2)
        {
            // Wrap back to start
            activeRow = 0;
            displayShift = 0;
        }
        else
        {
            activeRow++;
        }
        updateDisplayedChars();
        renderRow1();
    }

    // ── LEFT ─────────────────────────────────────────────────────────────────
    else if (leftPressed())
    {
        if (activeRow == 0 && displayShift > 0)
        {
            displayShift--;
        }
        else if (activeRow > -2)
        {
            activeRow--;
        }
        updateDisplayedChars();
        renderRow1();
    }

    // ── ENTER ────────────────────────────────────────────────────────────────
    if (enterPressed())
    {
        int len = strlen(typedText);
        if (activeRow == -2)
        {
            // OK — confirm
            confirmed = true;
        }
        else if (activeRow == -1)
        {
            // Space
            if (len < (int)sizeof(typedText) - 1)
            {
                typedText[len] = ' ';
                typedText[len + 1] = '\0';
            }
        }
        else
        {
            // Letter
            if (len < (int)sizeof(typedText) - 1)
            {
                typedText[len] = displayedChars[activeRow];
                typedText[len + 1] = '\0';
            }
        }

        // Refresh row 0 (typed text)
        lcd1.setCursor(0, 0);
        lcd1.print("                "); // clear 16 chars
        lcd1.setCursor(0, 0);
        lcd1.print(typedText);
    }

    // ── REMOVE ───────────────────────────────────────────────────────────────
    if (removePressed())
    {
        int len = strlen(typedText);
        if (len > 0)
        {
            typedText[len - 1] = '\0';
        }
        lcd1.setCursor(0, 0);
        lcd1.print("                ");
        lcd1.setCursor(0, 0);
        lcd1.print(typedText);
    }

    // ── BLINK ────────────────────────────────────────────────────────────────
    if (blinkLogic(400))
    {
        dipActive = !dipActive;
        renderRow1();
    }

    return confirmed ? typedText : "";
}