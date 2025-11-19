/**
 * MIDI BytePulse - Display Control
 */

#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

#include <Arduino.h>
#include <TM1637Display.h>

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
  TM1637Display display;
  bool beatState = false;
  uint8_t beatPosition = 0;  // Track which decimal to light (0-3)
};

#endif  // DISPLAY_CONTROL_H
