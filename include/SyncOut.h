/**
 * MIDI BytePulse - Sync Output Handler
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
  unsigned long getClockInterval() { return avgUSBClockInterval; }

private:
  void pulseClock();
  void pulseLED();
  void checkUSBTimeout();
  bool isJackConnected();
  
  unsigned long lastPulseTime = 0;
  unsigned long lastUSBClockTime = 0;
  unsigned long prevUSBClockTime = 0;
  unsigned long avgUSBClockInterval = 0;
  unsigned long lastDINClockTime = 0;
  unsigned long prevDINClockTime = 0;
  unsigned long avgDINClockInterval = 0;
  bool clockState = false;
  bool ledState = false;
  byte ppqnCounter = 0;
  bool isPlaying = false;
  bool usbIsPlaying = false;
  ClockSource activeSource = CLOCK_SOURCE_NONE;
};

#endif  // SYNC_OUT_H
