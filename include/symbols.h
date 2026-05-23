#ifndef SYMBOLS_H
#define SYMBOLS_H

void initSymbols();

// These values map directly to lcd.createChar() slot indices (0-7).
enum Symbols : byte
{
    PENCIL = 0,
    WARNING = 1,
    PERSON = 2,
    BELL = 3,
    STAR = 4,
    TIME_ICO = 5,
    CALENDAR = 6
};

#endif