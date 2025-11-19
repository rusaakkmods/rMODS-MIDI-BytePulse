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
}

void DisplayControl::showBPM(uint16_t bpm) {
  // Show BPM right-aligned (e.g., " 120")
  display.showNumberDec(bpm, false, 4, 0);
}

void DisplayControl::clear() {
  uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
  display.setSegments(data);
  beatPosition = 0;
}
