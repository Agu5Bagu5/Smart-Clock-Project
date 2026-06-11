#include <Arduino.h>
#include <Wire.h>
#include "data.h"

void writeEEPROM(int address, byte data)
{
    Wire.beginTransmission(0x57);
    Wire.write((int)(address >> 8));   // high byte
    Wire.write((int)(address & 0xFF)); // low byte
    Wire.write(data);
    Wire.endTransmission();
    delay(5); // AT24Cxx write cycle time
}

byte readEEPROM(int address)
{
    Wire.beginTransmission(0x57);
    Wire.write((int)(address >> 8));
    Wire.write((int)(address & 0xFF));
    Wire.endTransmission();

    Wire.requestFrom(0x57, 1);
    if (Wire.available())
    {
        return Wire.read();
    }
    return 0;
}