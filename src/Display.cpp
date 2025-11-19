/**
 * MIDI BytePulse - TM1637 7-Segment Display Implementation
 */

#include "Display.h"

Display::Display()
    : _display(TM1637_CLK_PIN, TM1637_DIO_PIN)
    , _lastBPM(0)
    , _needsUpdate(true) {
}

void Display::begin() {
    _display.setBrightness(0x0a); // Medium brightness (0-15)
    clear();
}

void Display::showBPM(uint16_t bpm) {
    if (bpm != _lastBPM) {
        _display.showNumberDec(bpm, false); // Show number, no leading zeros
        _lastBPM = bpm;
    }
}

void Display::showPPQN(uint8_t ppqn) {
    // Show PPQN right-aligned with leading "P"
    // Format: "P  6" or "P 24"
    uint8_t segments[] = {
        0b01110011, // P
        0x00,       // blank
        0x00,       // blank or first digit
        0x00        // second digit
    };
    
    if (ppqn >= 10) {
        segments[2] = _display.encodeDigit(ppqn / 10);
        segments[3] = _display.encodeDigit(ppqn % 10);
    } else {
        segments[3] = _display.encodeDigit(ppqn);
    }
    
    _display.setSegments(segments);
}

void Display::clear() {
    _display.clear();
    _lastBPM = 0;
}

void Display::setBrightness(uint8_t brightness) {
    // TM1637 uses 0-7 range
    if (brightness > 7) brightness = 7;
    _display.setBrightness(brightness);
}
