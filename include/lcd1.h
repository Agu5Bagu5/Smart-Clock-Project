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

    HOMEPAGE = 0,
    MENU = 1,
    ADD_SCHEDULE = 2,
    VIEW_SCHEDULE = 3,
    SUBJECTS = 4
};

extern State currentState;

void menu();
void addScheduleInterface();
void homepage();
void viewSchedules();
void subjects();

#endif