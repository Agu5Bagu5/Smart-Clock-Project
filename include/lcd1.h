#ifndef LCD1_H
#define LCD1_H

char *typing();
void lcd1Config();
void lcd1Main();

enum State
{
    ADD_SCHEDULE,
    HOMEPAGE,
    MENU,
};

extern State currentState;

// state
void menu();
void addScheduleInterface();

#endif
