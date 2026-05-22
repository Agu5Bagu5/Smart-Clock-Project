#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "constant.h"
#include "buttonPressLogic.h"
#include "blinkLogic.h"

int displayShift = 0;
int activeRow = 0; // Posisi kursor visual (0-11)

int dipInterval = 400;
bool dipActive = true;

const char alphabets[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
char displayedAlphabets[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};

void updateDisplayedAlphabets()
{
    for (int i = 0; i < 8; i++)
    {
        displayedAlphabets[i] = alphabets[i + displayShift];
    }
}

void updateDisplay()
{
    if (activeRow == -1 && !dipActive)
    {
        lcd1.setCursor(0, 1);
        lcd1.print("   Spc|");
    }
    else if (activeRow == -2 && !dipActive)
    {
        lcd1.setCursor(0, 1);
        lcd1.print("   Spc|");
    }
    else
    {
        lcd1.setCursor(0, 1);
        lcd1.print("Ok Spc|");
    }

    for (int i = 0; i < 12; i++)
    {
        lcd1.setCursor(i + 4, 1);
        if (i == activeRow && !dipActive)
        {
            lcd1.print("_");
        }
        else
        {
            lcd1.print(displayedAlphabets[i]);
        }
    }
}

//=============== LOGIKA TYPING ===============

char *typing()
{
    static char typedText[14] = "";
    static bool status = false;

    // RIGHT
    if (rightPressed())
    {
        if (activeRow >= 9 && displayShift < int(sizeof(alphabets) - sizeof(displayedAlphabets)))
        {
            displayShift++;
        }
        else if (activeRow < 11 && activeRow < int(sizeof(alphabets)))
        {
            activeRow++;
        }
        else
        {
            activeRow = 0;
            displayShift = 0;
        }

        updateDisplayedAlphabets();
        updateDisplay();
    }

    // LEFT
    else if (leftPressed())
    {
        if (activeRow <= 2 && displayShift != 0)
        {
            displayShift--;
        }
        else if (activeRow > -2)
        {
            activeRow--;
        }

        updateDisplayedAlphabets();
        updateDisplay();
    }

    // ENTER
    if (enterPressed())
    {
        if (strlen(typedText) < sizeof(typedText) - 1)
        {
            switch (activeRow)
            {
            case -2:
                status = true;
                break;
            case -1:
                strcat(typedText, " ");
                break;
            default:
                char temp[2] = {displayedAlphabets[activeRow], '\0'};
                strcat(typedText, temp);
                break;
            }

            lcd1.setCursor(0, 0);
            lcd1.print("                ");
            lcd1.setCursor(0, 0);
            lcd1.print(typedText);
        }
    }

    // REMOVE
    if (removePressed())
    {
        if (strlen(typedText) > 0)
        {
            typedText[strlen(typedText) - 1] = '\0';
        }

        lcd1.setCursor(0, 0);
        lcd1.print("                ");
        lcd1.setCursor(0, 0);
        lcd1.print(typedText);
    }

    // BLINK
    if (blinkLogic(dipInterval))
    {
        dipActive = !dipActive;
        updateDisplay();
    }

    static char result[14] = "";

    if (status)
    {
        strcpy(result, typedText);
    }

    return result;
}