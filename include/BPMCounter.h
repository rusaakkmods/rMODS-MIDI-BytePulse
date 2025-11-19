/**
 * MIDI BytePulse - BPM Counter
 * Measures tempo from MIDI clock pulses
 */

#ifndef BPM_COUNTER_H
#define BPM_COUNTER_H

#include <Arduino.h>

class DisplayControl;

class BPMCounter {
public:
  BPMCounter();
  void handleBeat();
  void reset();
  uint16_t getBPM();
  bool hasChanged(uint8_t threshold = 2);
  void setDisplayControl(DisplayControl* dc) { displayControl = dc; }
  void update();  // Call from main loop to handle beat off timing

private:
  uint8_t beatPosition = 0;  // Track beat position (0-3)
  unsigned long lastBeatTime = 0;
  unsigned long beatOnTime = 0;
  bool beatIsOn = false;
  uint16_t currentBPM = 0;
  uint16_t lastReportedBPM = 0;
  DisplayControl* displayControl = nullptr;
};

#endif  // BPM_COUNTER_H
