/**
 * MIDI BytePulse - Display Handler Implementation
 * Updates only when value changes - zero MIDI impact
 */

#include "Display.h"
#include "config.h"

void Display::begin() {
  tm1637 = new TM1637Display(DISPLAY_CLK_PIN, DISPLAY_DIO_PIN);
  tm1637->setBrightness(0x00);  // Minimum brightness (still visible)
  
  // Total animation: 2000ms
  // Segment cascade: 10 frames = 1000ms (100ms each)
  // Decimal points: 4 frames = 600ms (150ms each)
  // Blinks: 2 blinks = 400ms (100ms on + 100ms off per blink)
  
  uint8_t segments[7] = {
    0b00000001,  // Top
    0b00000010,  // Top right
    0b00000100,  // Bottom right
    0b00001000,  // Bottom
    0b00010000,  // Bottom left
    0b00100000,  // Top left
    0b01000000   // Middle
  };
  
  // Animate segments with cascade effect (10 frames total, 1000ms)
  // Digit 1 starts first, then digit 2, then 3, then 4 (each offset by 1 segment)
  for (int frame = 0; frame < 10; frame++) {
    uint8_t pattern[4];
    
    for (int digit = 0; digit < 4; digit++) {
      // Each digit is offset by its position
      int segmentIdx = frame - digit;
      
      if (segmentIdx >= 0 && segmentIdx < 7) {
        pattern[digit] = segments[segmentIdx];
      } else {
        pattern[digit] = 0b00000000;  // Off if not in range
      }
    }
    
    tm1637->setSegments(pattern);
    delay(100);  // 10 * 100ms = 1000ms
  }
  
  // Animate decimal points (4 frames, 600ms total)
  for (int dp = 0; dp < 4; dp++) {
    uint8_t pattern[4] = {0b00000000, 0b00000000, 0b00000000, 0b00000000};
    pattern[dp] = 0b10000000;
    tm1637->setSegments(pattern);
    delay(150);  // 4 * 150ms = 600ms
  }
  
  // Blink all segments 2 times (400ms total)
  for (int blink = 0; blink < 2; blink++) {
    // All on
    uint8_t allOn[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    tm1637->setSegments(allOn);
    delay(100);
    
    // All off
    uint8_t allOff[4] = {0x00, 0x00, 0x00, 0x00};
    tm1637->setSegments(allOff);
    delay(100);
  }
}

void Display::showStandby() {
  if (tm1637) {
    // Show "StbY"
    // S = 0b01101101, t = 0b01111000, b = 0b01111100, Y = 0b01101110
    uint8_t stby[] = {0b01101101, 0b01111000, 0b01111100, 0b01101110};
    tm1637->setSegments(stby);
    lastClockState = false;
  }
}

void Display::updateClockIndicator(bool clockRunning) {
  // Only update when clock state changes (start/stop)
  if (!tm1637 || clockRunning == lastClockState) return;
  
  lastClockState = clockRunning;
  
  // Show decimal point on 4th digit when clock is running
  uint8_t pattern[4] = {0x00, 0x00, 0x00, static_cast<uint8_t>(clockRunning ? 0x80 : 0x00)};
  tm1637->setSegments(pattern);
}

void Display::setBPM(uint16_t bpm) {
  // Constrain to valid range and only update if different
  bpm = constrain(bpm, 20, 400);
  if (bpm != currentBPM) {
    currentBPM = bpm;
    needsUpdate = true;
  }
}

void Display::setSource(const char* source) {
  // Optional: Show source indicator
  // Could use leftmost digit: U=USB, d=DIN, S=Sync
  // For now, just show BPM
}

void Display::clear() {
  if (tm1637) {
    tm1637->clear();
    currentBPM = 0;
  }
}

void Display::updateIfNeeded() {
  if (needsUpdate && tm1637) {
    // Update display ONLY when BPM changes
    // Takes ~2ms but happens rarely (once per second max)
    tm1637->showNumberDec(currentBPM, false);
    needsUpdate = false;
  }
}

// Global helper function for main loop
extern Display display;
void displayUpdateHelper() {
  display.updateIfNeeded();
}
