#include <Arduino.h>
#include <Wire.h>
#include <data.h>

void writeEEPROM(int address, byte data)
{
    Wire.beginTransmission(0x50); // Alamat I2C untuk EEPROM

    Wire.write((int)(address >> 8));   // Alamat tinggi
    Wire.write((int)(address & 0xFF)); // Alamat rendah

    Wire.write(data); // Data yang akan ditulis
    Wire.endTransmission();

    delay(5); // Waktu tunggu untuk memastikan data tertulis
}

byte readEEPROM(int address)
{
    Wire.beginTransmission(0x50); // Alamat I2C untuk EEPROM

    Wire.write((int)(address >> 8));   // Alamat tinggi
    Wire.write((int)(address & 0xFF)); // Alamat rendah
    Wire.endTransmission();

    Wire.requestFrom(0x50, 1); // Minta 1 byte data

    if (Wire.available())
    {
        return Wire.read(); // Baca data dari EEPROM
    }
    return 0; // Kembalikan nilai default jika tidak ada data yang tersedia
}