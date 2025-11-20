/**
 * MIDI BytePulse - Display Handler
 * Zero-impact TM1637 display management
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TM1637Display.h>

class Display {
public:
  void begin();
  void showStandby();
  void updateBeatIndicator(bool beatActive);
  void setBPM(uint16_t bpm);
  void setSource(const char* source);
  void clear();

private:
  TM1637Display* tm1637 = nullptr;
  uint16_t currentBPM = 0;
  bool needsUpdate = false;
  bool needsInit = false;
  bool lastBeatState = false;
  
  void initializeHardware();
  void updateIfNeeded();
  friend void displayUpdateHelper();
};

#endif  // DISPLAY_H
