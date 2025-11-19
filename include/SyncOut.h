/**
 * MIDI BytePulse - Sync Output Handler
 * Generates clock sync pulses and beat LED indicator
 */

#ifndef SYNC_OUT_H
#define SYNC_OUT_H

#include <Arduino.h>

enum ClockSource {
  CLOCK_SOURCE_NONE,
  CLOCK_SOURCE_DIN,
  CLOCK_SOURCE_USB
};

class SyncOut {
public:
  void begin();
  void handleClock(ClockSource source);
  void handleStart(ClockSource source);
  void handleStop(ClockSource source);
  void update();
  ClockSource getActiveSource() { return activeSource; }

private:
  void pulseClock();
  void pulseLED();
  
  unsigned long lastPulseTime = 0;
  unsigned long lastUSBClockTime = 0;
  bool clockState = false;
  bool ledState = false;
  byte ppqnCounter = 0;
  bool isPlaying = false;
  bool usbIsPlaying = false;
  ClockSource activeSource = CLOCK_SOURCE_NONE;
};

#endif  // SYNC_OUT_H
