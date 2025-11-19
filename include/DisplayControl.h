/**
 * MIDI BytePulse - Display Control
 */

#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

#include <Arduino.h>
#include <TM1637TinyDisplay.h>

class DisplayControl {
public:
  DisplayControl();
  void begin();
  void beatOn();
  void beatOff();
  void showStopIndicator();
  void showBPM(uint16_t bpm);
  void clear();

private:
  TM1637TinyDisplay display;
  bool beatState = false;
  uint8_t beatPosition = 0;  // Track which decimal to light (0-3)
  uint8_t currentSegments[4] = {0x00, 0x00, 0x00, 0x00};  // Track current display state
};

#endif  // DISPLAY_CONTROL_H
