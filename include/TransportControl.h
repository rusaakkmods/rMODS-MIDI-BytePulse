/**
 * MIDI BytePulse - Transport Control
 */

#ifndef TRANSPORT_CONTROL_H
#define TRANSPORT_CONTROL_H

#include <Arduino.h>

class HC595Display;
class BPMCounter;

enum TransportState {
  TRANSPORT_STOP,
  TRANSPORT_PLAY,
  TRANSPORT_PAUSE
};

class TransportControl {
public:
  void begin();
  void update();
  TransportState getState() { return state; }
  void setDisplay(HC595Display* disp) { display = disp; }
  void setBPMCounter(BPMCounter* counter) { bpmCounter = counter; }
  bool isMessageDisplayActive() const { return messageDisplayActive; }

private:
  void handlePlayButton();
  void handleStopButton();
  void sendStart();
  void sendContinue();
  void sendStop();
  
  TransportState state = TRANSPORT_STOP;
  bool lastPlayState = HIGH;
  bool lastStopState = HIGH;
  bool playPressed = false;
  bool stopPressed = false;
  unsigned long lastPlayDebounce = 0;
  unsigned long lastStopDebounce = 0;
  unsigned long messageDisplayTime = 0;
  bool messageDisplayActive = false;
  
  HC595Display* display = nullptr;
  BPMCounter* bpmCounter = nullptr;
};

#endif  // TRANSPORT_CONTROL_H
