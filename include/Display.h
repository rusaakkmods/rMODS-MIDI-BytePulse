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
  void showClockIndicator();  // Show "0." when clock starts
  void flush();                // Non-blocking incremental flush - call every loop

private:
  ace_tmi::SimpleTmi1637Interface* tmiInterface = nullptr;
  ace_segment::Tm1637Module<ace_tmi::SimpleTmi1637Interface, 4>* ledModule = nullptr;
  uint16_t currentBPM = 0;
  unsigned long lastFlushTime = 0;
  
  void initializeHardware();
};

#endif  // DISPLAY_H
