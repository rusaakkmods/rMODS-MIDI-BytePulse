/**
 * 74HC595 Dual Shift Register 7-Segment Display Driver
 * Bit-bang multiplexing for non-blocking operation
 */

#include "HC595Display.h"
#include "config.h"

// 7-Segment patterns for digits 0-9 (common cathode)
// Bit order: DP G F E D C B A
const uint8_t HC595Display::DIGIT_PATTERNS[10] = {
    0xC0,  // 0
    0xF9,  // 1
    0xA4,  // 2
    0xB0,  // 3
    0x99,  // 4
    0x92,  // 5
    0x82,  // 6
    0xF8,  // 7
    0x80,  // 8
    0x90   // 9
};

// Special character patterns
const uint8_t HC595Display::CHAR_T = 0x87;  // 't' (segments D,E,F,G)
const uint8_t HC595Display::CHAR_V = 0xE3;  // 'u' (segments C,D,E) - lower u shape
const uint8_t HC595Display::CHAR_P = 0x8C;  // 'P' (segments A,B,E,F,G)
const uint8_t HC595Display::CHAR_DASH = 0xBF;  // '-' (segment G only)

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

void HC595Display::showBPM(uint16_t bpm) {
    if (bpm > 999) bpm = 999;  // Max 3 digits
    
    // First digit shows "t" for tempo
    _displayBuffer[0] = CHAR_T;
    
    // Right-align the BPM value in remaining 3 digits
    if (bpm < 10) {
        // Single digit: "t  X"
        _displayBuffer[1] = 0xFF;  // Blank
        _displayBuffer[2] = 0xFF;  // Blank
        _displayBuffer[3] = DIGIT_PATTERNS[bpm];
    } else if (bpm < 100) {
        // Two digits: "t XX"
        _displayBuffer[1] = 0xFF;  // Blank
        _displayBuffer[2] = DIGIT_PATTERNS[bpm / 10];
        _displayBuffer[3] = DIGIT_PATTERNS[bpm % 10];
    } else {
        // Three digits: "tXXX"
        _displayBuffer[1] = DIGIT_PATTERNS[bpm / 100];
        _displayBuffer[2] = DIGIT_PATTERNS[(bpm / 10) % 10];
        _displayBuffer[3] = DIGIT_PATTERNS[bpm % 10];
    }
}

void HC595Display::showStopped() {
    // Show "-  ." when stopped (dash in first digit, decimal in last)
    _displayBuffer[0] = CHAR_DASH;
    _displayBuffer[1] = 0xFF;  // Blank
    _displayBuffer[2] = 0xFF;  // Blank
    _displayBuffer[3] = 0xFF;  // Blank (decimal will be set separately)
}

void HC595Display::showVolume(uint8_t volume) {
    if (volume > 127) volume = 127;  // MIDI max
    
    // First digit shows "v" for volume
    _displayBuffer[0] = CHAR_V;
    
    // Right-align the volume value in remaining 3 digits
    if (volume < 10) {
        // Single digit: "v  X"
        _displayBuffer[1] = 0xFF;  // Blank
        _displayBuffer[2] = 0xFF;  // Blank
        _displayBuffer[3] = DIGIT_PATTERNS[volume];
    } else if (volume < 100) {
        // Two digits: "v XX"
        _displayBuffer[1] = 0xFF;  // Blank
        _displayBuffer[2] = DIGIT_PATTERNS[volume / 10];
        _displayBuffer[3] = DIGIT_PATTERNS[volume % 10];
    } else {
        // Three digits: "vXXX"
        _displayBuffer[1] = DIGIT_PATTERNS[volume / 100];
        _displayBuffer[2] = DIGIT_PATTERNS[(volume / 10) % 10];
        _displayBuffer[3] = DIGIT_PATTERNS[volume % 10];
    }
}

void HC595Display::showPitch(uint8_t pitch) {
    if (pitch > 127) pitch = 127;  // MIDI max
    
    // First digit shows "P" for pitch
    _displayBuffer[0] = CHAR_P;
    
    // Right-align the pitch value in remaining 3 digits
    if (pitch < 10) {
        // Single digit: "P  X"
        _displayBuffer[1] = 0xFF;  // Blank
        _displayBuffer[2] = 0xFF;  // Blank
        _displayBuffer[3] = DIGIT_PATTERNS[pitch];
    } else if (pitch < 100) {
        // Two digits: "P XX"
        _displayBuffer[1] = 0xFF;  // Blank
        _displayBuffer[2] = DIGIT_PATTERNS[pitch / 10];
        _displayBuffer[3] = DIGIT_PATTERNS[pitch % 10];
    } else {
        // Three digits: "PXXX"
        _displayBuffer[1] = DIGIT_PATTERNS[pitch / 100];
        _displayBuffer[2] = DIGIT_PATTERNS[(pitch / 10) % 10];
        _displayBuffer[3] = DIGIT_PATTERNS[pitch % 10];
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
