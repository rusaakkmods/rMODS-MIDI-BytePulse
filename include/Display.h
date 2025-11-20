/**
 * MIDI BytePulse - Display Handler
 * Non-blocking TM1637 display management using AceSegment
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <AceTMI.h>
#include <AceSegment.h>

class Display {
public:
  void begin();
  void showStandby();
  void updateClockIndicator(bool clockRunning);
  void setBPM(uint16_t bpm);
  void setSource(const char* source);
  void clear();
  void flush();                // Non-blocking incremental flush - call every loop
  void showMIDIMessage(const char* type, uint8_t data, uint8_t channel = 0);  // Show brief MIDI message
  void showPlay();  // Show "PlaY" briefly
  void showStop();  // Show "StoP" briefly
  void advanceAnimation();  // Advance play animation one step (call on MIDI clock)
  void setBeat(uint8_t beat);  // Set current beat position (0-3)

private:
  ace_tmi::SimpleTmi1637Interface* tmiInterface = nullptr;
  ace_segment::Tm1637Module<ace_tmi::SimpleTmi1637Interface, 4>* ledModule = nullptr;
  uint16_t currentBPM = 0;
  unsigned long lastFlushTime = 0;
  bool isIdle = false;
  bool isPlaying = false;
  unsigned long lastIdleAnimTime = 0;
  uint8_t idleAnimFrame = 0;
  unsigned long midiMessageTime = 0;
  bool showingMIDIMessage = false;
  bool animationNeedsUpdate = false;
  uint8_t currentBeat = 0;  // Track current beat (0-3) for decimal display
  
  void initializeHardware();
  uint8_t charToSegment(char c);
};

#endif  // DISPLAY_H
