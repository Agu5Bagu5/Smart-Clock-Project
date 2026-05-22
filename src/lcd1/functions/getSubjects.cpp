#include <Arduino.h>
#include <data.h>
#include "time-date.h"

static int adress(int e, int index)
{
    return ((0 + e) + 16 * index);
}

void getSubjects()
{
    static int RAMIndex = 0;

    for (int i = 0; i < 50; i++)
    {
        byte category = readEEPROM(adress(1, i));

        if (category == -2)
        {
            break;
        }

        if (category != -1)
        {
            subjects[RAMIndex].category = category;
            subjects[RAMIndex].subjectId = readEEPROM(adress(2, i));
            for (int e = 0; e < 14; e++)
            {
                subjects[RAMIndex].subject[e] = readEEPROM(adress(3, i) + e);
            }
        }
    }
}
