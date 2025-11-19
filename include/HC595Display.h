/**
 * 74HC595 Dual Shift Register 7-Segment Display Driver
 * Minimal implementation for beat indicator only
 */

#ifndef HC595_DISPLAY_H
#define HC595_DISPLAY_H

#include <Arduino.h>

class HC595Display {
public:
    HC595Display(uint8_t latchPin);
    
    void begin();
    
    // Beat indicator
    void setDecimalPoint(uint8_t position, bool on);
    
    // Multiplexing (call in main loop)
    void updateDisplay();
    
    // Public for direct access
    uint8_t _displayBuffer[4];  // Segment data for each digit
    
private:
    uint8_t _latchPin;
    
    void shiftOutBitBang(uint8_t data);
};

#endif // HC595_DISPLAY_H
