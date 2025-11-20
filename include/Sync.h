/**
 * MIDI BytePulse - Sync Handler
 */

#ifndef SYNC_H
#define SYNC_H

#include <Arduino.h>

class Display;  // Forward declaration

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
  uint16_t getCurrentBPM() const { return currentBPM; }
  
  // BPM update callback - set this to update display
  void (*onBPMUpdate)(uint16_t bpm) = nullptr;
  void (*onClockStop)() = nullptr;
  void (*onClockStart)() = nullptr;
  
  void setDisplay(Display* disp) { display = disp; }

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
  
  // BPM calculation
  byte beatPosition = 0;
  unsigned long lastBeatTime = 0;
  uint16_t currentBPM = 0;
  uint16_t lastDisplayedBPM = 0;  // For change detection
  
  Display* display = nullptr;  // Display reference for animation sync
};

#endif  // SYNC_H
