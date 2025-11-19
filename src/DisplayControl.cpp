/**
 * MIDI BytePulse - Display Control Implementation
 */

#include "DisplayControl.h"
#include "config.h"

DisplayControl::DisplayControl() : display(TM1637_CLK_PIN, TM1637_DIO_PIN) {
}

void DisplayControl::begin() {
  display.setBrightness(7);  // TM1637TinyDisplay uses 0-7 brightness
  showStopIndicator();
}

void DisplayControl::beatOn() {
  if (!beatState) {
    // Add decimal point to current position (bit 7 = 0x80)
    currentSegments[beatPosition] |= 0x80;
    // Update only the current position (faster - single digit update)
    display.setSegments(&currentSegments[beatPosition], 1, beatPosition);
    
    // Move to next position for next beat
    beatPosition = (beatPosition + 1) % 4;
    beatState = true;
  }
}

void DisplayControl::beatOff() {
  if (beatState) {
    // Remove decimal point from previous position
    uint8_t prevPosition = (beatPosition + 3) % 4;  // Go back one position
    currentSegments[prevPosition] &= ~0x80;
    // Update only the previous position (faster - single digit update)
    display.setSegments(&currentSegments[prevPosition], 1, prevPosition);
    beatState = false;
  }
}

void DisplayControl::showStopIndicator() {
  // Display "   -" (dash in last position)
  currentSegments[0] = 0x00;
  currentSegments[1] = 0x00;
  currentSegments[2] = 0x00;
  currentSegments[3] = 0x40;
  display.setSegments(currentSegments);
  beatState = false;
  beatPosition = 0;  // Reset to first position when stopped
}

void DisplayControl::showBPM(uint16_t bpm) {
  // Show BPM right-aligned (e.g., " 120")
  display.showNumberDec(bpm, false, 4, 0);
  // Note: This will overwrite currentSegments tracking, but that's okay
  // since we're changing display mode entirely
}

void DisplayControl::clear() {
  currentSegments[0] = 0x00;
  currentSegments[1] = 0x00;
  currentSegments[2] = 0x00;
  currentSegments[3] = 0x00;
  display.setSegments(currentSegments);
  beatPosition = 0;
}
