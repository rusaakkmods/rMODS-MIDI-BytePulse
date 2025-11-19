/**
 * 74HC595 Dual Shift Register 7-Segment Display Driver
 * Minimal implementation for BPM display and beat indicator
 */

#ifndef HC595_DISPLAY_H
#define HC595_DISPLAY_H

#include <Arduino.h>

class HC595Display {
public:
    HC595Display(uint8_t latchPin);
    
    void begin();
    
    // Display control
    void showBPM(uint16_t bpm);  // Show BPM with "t" prefix
    void showStopped();          // Show "-" when stopped
    void showVolume(uint8_t volume);  // Show volume with "v" prefix
    void showPitch(uint8_t pitch);    // Show pitch with "P" prefix
    
    // Beat indicator
    void setDecimalPoint(uint8_t position, bool on);
    
    // Multiplexing (call in main loop)
    void updateDisplay();
    
    // Public for direct access
    uint8_t _displayBuffer[4];  // Segment data for each digit
    
private:
    uint8_t _latchPin;
    
    static const uint8_t DIGIT_PATTERNS[10];
    static const uint8_t CHAR_T;     // 't' character
    static const uint8_t CHAR_V;     // 'v' character  
    static const uint8_t CHAR_P;     // 'P' character
    static const uint8_t CHAR_DASH;  // '-' character
    
    void shiftOutBitBang(uint8_t data);
};

#endif // HC595_DISPLAY_H
