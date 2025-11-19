/**
 * 74HC595 Dual Shift Register 7-Segment Display Driver
 * Bit-bang multiplexing for non-blocking operation
 */

#include "HC595Display.h"
#include "config.h"

HC595Display::HC595Display(uint8_t latchPin) 
    : _latchPin(latchPin) {
    for (uint8_t i = 0; i < 4; i++) {
        _displayBuffer[i] = 0xFF;  // All segments off
    }
}

void HC595Display::begin() {
    pinMode(_latchPin, OUTPUT);
    pinMode(DISPLAY_DIO_PIN, OUTPUT);
    pinMode(DISPLAY_SCLK_PIN, OUTPUT);
    digitalWrite(_latchPin, LOW);
}

void HC595Display::shiftOutBitBang(uint8_t data) {
    for (int i = 7; i >= 0; i--) {
        digitalWrite(DISPLAY_SCLK_PIN, LOW);
        digitalWrite(DISPLAY_DIO_PIN, (data >> i) & 0x01);
        digitalWrite(DISPLAY_SCLK_PIN, HIGH);
    }
}

void HC595Display::setDecimalPoint(uint8_t position, bool on) {
    if (position > 3) return;
    
    if (on) {
        _displayBuffer[position] &= 0x7F;  // Clear bit = ON
    } else {
        _displayBuffer[position] |= 0x80;  // Set bit = OFF
    }
}

void HC595Display::updateDisplay() {
    // Rapid multiplexing - call this frequently from main loop
    // Digit mapping from example: 1=8, 2=4, 3=2, 4=1
    static uint8_t currentDigit = 0;
    static const uint8_t digitBits[4] = {8, 4, 2, 1};  // Digit 0,1,2,3 mapping
    
    // Send segments first, digit select second
    digitalWrite(_latchPin, LOW);
    shiftOutBitBang(_displayBuffer[currentDigit]);
    shiftOutBitBang(digitBits[currentDigit]);
    digitalWrite(_latchPin, HIGH);
    
    currentDigit = (currentDigit + 1) & 0x03;  // Wrap 0-3
}
