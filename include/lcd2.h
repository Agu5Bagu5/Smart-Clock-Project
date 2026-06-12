#ifndef LCD2_H
#define LCD2_H

// clockDisplay() — reads RTC, updates lcd2, fires midnight hooks.
// Call via lcd2Main() only; do not call directly.
void clockDisplay();

// lcd2Main() — thin wrapper called from loop().
// BUG FIX: this declaration was removed during reminder system integration,
// leaving the function undefined.  On AVR, calling an undefined function
// typically falls through to address 0 (the RESET vector), causing the MCU
// to reset on every loop() iteration — which is exactly the "both displays
// blank / reset on startup" symptom that was observed.
void lcd2Main();

#endif