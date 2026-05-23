#ifndef LCD1_H
#define LCD1_H

// Returns pointer to completed string when user confirms, "" otherwise.
// Resets internal state when resetTyping() is called.
const char *typing();
void resetTyping();

void lcd1Config();
void lcd1Main();

enum State : byte
{
    ADD_SCHEDULE = 0,
    HOMEPAGE,
    MENU,
};

extern State currentState;

void menu();
void addScheduleInterface();
void homepage();

#endif