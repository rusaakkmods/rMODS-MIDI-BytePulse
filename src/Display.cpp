/**
 * MIDI BytePulse - Display Handler Implementation
 * Non-blocking updates using AceSegment library
 */

#include "Display.h"
#include "config.h"

using ace_tmi::SimpleTmi1637Interface;
using ace_segment::Tm1637Module;

void Display::begin() {
  // Create TM1637 interface with 100us bit delay - guaranteed to work with all modules
  // Even with capacitors, this is the safest setting from AceSegment examples
  tmiInterface = new SimpleTmi1637Interface(DISPLAY_DIO_PIN, DISPLAY_CLK_PIN, 100);
  ledModule = new Tm1637Module<SimpleTmi1637Interface, 4>(*tmiInterface);
  
  tmiInterface->begin();
  ledModule->begin();
  ledModule->setBrightness(2);  // Medium brightness (0-7)
  
  // Total animation: 2000ms - matching original behavior
  uint8_t segments[7] = {
    0b00000001,  // Top
    0b00000010,  // Top right
    0b00000100,  // Bottom right
    0b00001000,  // Bottom
    0b00010000,  // Bottom left
    0b00100000,  // Top left
    0b01000000   // Middle
  };
  
  // Segment cascade (10 frames, 1000ms)
  for (int frame = 0; frame < 10; frame++) {
    for (int digit = 0; digit < 4; digit++) {
      int segmentIdx = frame - digit;
      if (segmentIdx >= 0 && segmentIdx < 7) {
        ledModule->setPatternAt(digit, segments[segmentIdx]);
      } else {
        ledModule->setPatternAt(digit, 0b00000000);
      }
    }
    ledModule->flush();
    delay(100);
  }
  
  // Decimal points (4 frames, 600ms)
  for (int dp = 0; dp < 4; dp++) {
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, digit == dp ? 0b10000000 : 0b00000000);
    }
    ledModule->flush();
    delay(150);
  }
  
  // Blink all (2 blinks, 400ms)
  for (int blink = 0; blink < 2; blink++) {
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, 0xFF);
    }
    ledModule->flush();
    delay(100);
    
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, 0x00);
    }
    ledModule->flush();
    delay(100);
  }
}

void Display::showStandby() {
  if (ledModule) {
    // Show "StbY"
    uint8_t stby[] = {0b01101101, 0b01111000, 0b01111100, 0b01101110};
    for (int i = 0; i < 4; i++) {
      ledModule->setPatternAt(i, stby[i]);
    }
    ledModule->flush();
  }
}

void Display::updateClockIndicator(bool clockRunning) {
  // Not used with AceSegment - decimal is part of BPM pattern
}

void Display::setBPM(uint16_t bpm) {
  bpm = constrain(bpm, 20, 400);
  
  // Only update if changed by more than 2 BPM
  if (abs((int)bpm - (int)currentBPM) > 2) {
    currentBPM = bpm;
    
    if (ledModule) {
      // Convert BPM to 3 digits (right-aligned)
      uint8_t hundreds = (bpm / 100) % 10;
      uint8_t tens = (bpm / 10) % 10;
      uint8_t ones = bpm % 10;
      
      // Digit patterns for 0-9
      const uint8_t digitToSegment[10] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111  // 9
      };
      
      // Right-align: blank, hundreds (or blank if 0), tens, ones with decimal
      ledModule->setPatternAt(0, 0b00000000);  // First digit always blank
      ledModule->setPatternAt(1, hundreds > 0 ? digitToSegment[hundreds] : 0b00000000);
      ledModule->setPatternAt(2, digitToSegment[tens]);
      ledModule->setPatternAt(3, digitToSegment[ones] | 0b10000000);  // Ones with decimal point
      
      // Patterns are set, flushIncremental() will handle the update
    }
  }
}

void Display::showClockIndicator() {
  // Show "  0." when clock starts
  if (ledModule) {
    ledModule->setPatternAt(0, 0b00000000);
    ledModule->setPatternAt(1, 0b00000000);
    ledModule->setPatternAt(2, 0b00000000);
    ledModule->setPatternAt(3, 0b00111111 | 0b10000000);  // 0 with decimal
    currentBPM = 0;
  }
}

void Display::setSource(const char* source) {
  // Optional: Show source indicator
}

void Display::clear() {
  if (ledModule) {
    for (int i = 0; i < 4; i++) {
      ledModule->setPatternAt(i, 0b00000000);
    }
    ledModule->flush();
    currentBPM = 0;
  }
}

void Display::flush() {
  // Call flushIncremental() every 20ms like the AceSegment examples
  // This updates one digit at a time in a round-robin fashion
  if (ledModule) {
    unsigned long now = millis();
    if ((unsigned long)(now - lastFlushTime) >= 20) {
      lastFlushTime = now;
      ledModule->flushIncremental();
    }
  }
}
