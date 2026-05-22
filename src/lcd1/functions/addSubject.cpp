#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "lcd1.h"
#include "data.h"
#include "constant.h"
#include "blinkLogic.h"

static int adress(int e, int index)
{
    return ((0 + e) + 16 * index);
}

void addSubject(byte category, byte subjectId, char *subject)
{
    for (int i = 0; i < 200; i++)
    {
        byte categoryRead = readEEPROM(adress(1, i));
        if (categoryRead == -1 || categoryRead == -2)
        {
            if (categoryRead == -2)
            {
                writeEEPROM(adress(6, i) + 16, -2);
            }

            if (categoryRead != category)
            {
                writeEEPROM(adress(1, i), category);
            }

            writeEEPROM(adress(2, i), subjectId);

            for (int e = 0; e < 14; e++)
            {
                writeEEPROM(adress(3, i) + e, subject[e]);
            }

            getSubjects();
            break;
        }
    }
}
