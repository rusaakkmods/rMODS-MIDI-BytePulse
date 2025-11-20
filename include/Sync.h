/**
 * MIDI BytePulse - Sync Handler
 */

#ifndef SYNC_H
#define SYNC_H

#include <Arduino.h>

enum ClockSource {
  CLOCK_SOURCE_NONE,
  CLOCK_SOURCE_SYNC_IN,
  CLOCK_SOURCE_DIN,
  CLOCK_SOURCE_USB
};

class Sync {
public:
  void begin();
  void handleClock(ClockSource source);
  void handleStart(ClockSource source);
  void handleStop(ClockSource source);
  void handleSyncInPulse();
  void update();
  bool isBeatActive() const { return ledState; }
  bool isClockRunning() const { return isPlaying; }

private:
  void checkUSBTimeout();
  bool isSyncOutConnected();
  bool isSyncInConnected();
  void sendMIDIClock();
  
  unsigned long lastPulseTime = 0;
  unsigned long lastUSBClockTime = 0;
  unsigned long prevUSBClockTime = 0;
  unsigned long avgUSBClockInterval = 0;
  unsigned long lastDINClockTime = 0;
  unsigned long prevDINClockTime = 0;
  unsigned long avgDINClockInterval = 0;
  unsigned long lastSyncInTime = 0;
  unsigned long prevSyncInTime = 0;
  unsigned long avgSyncInInterval = 0;
  volatile unsigned long syncInPulseTime = 0;
  bool clockState = false;
  bool ledState = false;
  byte ppqnCounter = 0;
  bool isPlaying = false;
  bool usbIsPlaying = false;
  bool syncInIsPlaying = false;
  ClockSource activeSource = CLOCK_SOURCE_NONE;
};

#endif  // SYNC_H
