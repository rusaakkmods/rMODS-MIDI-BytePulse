/**
 * MIDI BytePulse - Display Control Implementation
 */

#include "DisplayControl.h"
#include "config.h"

DisplayControl::DisplayControl() : display(TM1637_CLK_PIN, TM1637_DIO_PIN) {
}

void DisplayControl::begin() {
  display.setBrightness(0x0f);
  showStopIndicator();
}

void DisplayControl::beatOn() {
  if (!beatState) {
    // Show decimal point rotating through positions (bit 7 = 0x80)
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    data[beatPosition] = 0x80;
    display.setSegments(data);
    
    // When back to first position, calculate BPM
    if (beatPosition == 0) {
      unsigned long now = millis();
      if (lastBeatTime > 0) {
        unsigned long interval = now - lastBeatTime;
        // Interval is for 4 beats (one measure), so divide by 4 to get per-beat interval
        // BPM = 60,000 ms/min / (interval_ms / 4)
        // Simplified: BPM = 240,000 / interval_ms
        currentBPM = 240000UL / interval;
        
        #if SERIAL_DEBUG
        DEBUG_PRINT(interval);
        DEBUG_PRINT("ms = ");
        DEBUG_PRINT(currentBPM);
        DEBUG_PRINTLN(" BPM");
        #endif
      }
      lastBeatTime = now;
    }
    
    // Move to next position for next beat
    beatPosition = (beatPosition + 1) % 4;
    beatState = true;
  }
}

void DisplayControl::beatOff() {
  if (beatState) {
    // Clear all segments
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    display.setSegments(data);
    beatState = false;
  }
}

void DisplayControl::showStopIndicator() {
  // Display "   -" (dash in last position)
  uint8_t data[] = {0x00, 0x00, 0x00, 0x40};
  display.setSegments(data);
  beatState = false;
  beatPosition = 0;  // Reset to first position when stopped
  
  #if SERIAL_DEBUG
  if (currentBPM > 0) {
    DEBUG_PRINT("Last BPM: ");
    DEBUG_PRINTLN(currentBPM);
  }
  #endif
}

void DisplayControl::reset() {
  beatPosition = 0;  // Reset to first position
  lastBeatTime = 0;  // Reset timing for next measurement
  // Don't reset currentBPM - keep last known value
}

uint16_t DisplayControl::getBPM() {
  return currentBPM;
}
