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
  void updateClockIndicator(bool clockRunning);
  void setBPM(uint16_t bpm);
  void setSource(const char* source);
  void clear();
  void showWaiting();  // Show blinking "---" while calculating BPM
  void showClockIndicator();  // Show just the clock indicator
  void update();       // Call in main loop for blinking

private:
  TM1637Display* tm1637 = nullptr;
  uint16_t currentBPM = 0;
  bool needsUpdate = false;
  bool needsInit = false;
  bool lastClockState = false;
  bool isWaiting = false;
  unsigned long lastBlinkTime = 0;
  bool blinkState = false;
  
  void initializeHardware();
  void updateIfNeeded();
  friend void displayUpdateHelper();
};

#endif  // DISPLAY_H
