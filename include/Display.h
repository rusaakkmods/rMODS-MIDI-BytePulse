#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <AceTMI.h>
#include <AceSegment.h>

class Display {
public:
  void begin();
  void setBPM(uint16_t bpm);
  void clear();
  void flush();
  void showMIDIMessage(const char* type, uint8_t data, uint8_t channel = 0);
  void showPlay();
  void showStop();
  void advanceAnimation();
  void setBeat(uint8_t beat);
  void showBPM();
  void showIdle();
  void setButtonPressed(bool pressed);

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
  uint8_t currentBeat = 0;
  bool buttonPressed = false;
  
  void initializeHardware();
  uint8_t charToSegment(char c);
};

#endif
