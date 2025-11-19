/**
 * MIDI BytePulse - BPM Counter
 * Measures tempo from MIDI clock pulses
 */

#ifndef BPM_COUNTER_H
#define BPM_COUNTER_H

#include <Arduino.h>

class HC595Display;
class PotControl;

class BPMCounter {
public:
  BPMCounter();
  void handleBeat();
  void reset();
  void start();  // Called when playback starts
  uint16_t getBPM();
  bool isPlaying() { return currentBPM > 0; }  // Check if currently playing
  bool hasChanged(uint8_t threshold = 2);
  void setDisplay(HC595Display* disp) { display = disp; }
  void setPotControl(PotControl* pots) { potControl = pots; }
  void update();  // Call from main loop to handle beat off timing and BPM display

private:
  uint8_t beatPosition = 0;  // Track beat position (0-3) for calculation timing
  unsigned long lastBeatTime = 0;
  unsigned long capturedInterval = 0;  // Captured interval for deferred calculation
  unsigned long beatOnTime = 0;
  bool beatIsOn = false;
  bool beatNeedsOn = false;   // Flag to turn on beat in main loop
  bool beatNeedsOff = false;  // Flag to turn off beat in main loop
  bool bpmNeedsCalculation = false;  // Flag to calculate BPM in main loop
  uint16_t currentBPM = 0;
  uint16_t displayedBPM = 0;  // Track what BPM is currently shown
  uint16_t pendingBPM = 0;    // Pending BPM to display
  bool bpmNeedsUpdate = false;
  HC595Display* display = nullptr;
  PotControl* potControl = nullptr;
};

#endif  // BPM_COUNTER_H
