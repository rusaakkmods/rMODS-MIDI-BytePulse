/**
 * MIDI BytePulse - Potentiometer Control
 */

#ifndef POT_CONTROL_H
#define POT_CONTROL_H

#include <Arduino.h>

class HC595Display;
class BPMCounter;

class PotControl {
public:
  void begin();
  void update();
  void setDisplay(HC595Display* disp) { display = disp; }
  void setBPMCounter(BPMCounter* counter) { bpmCounter = counter; }
  bool isVolumeDisplayActive() const { return volumeDisplayActive; }

private:
  void readVolume();
  void readPitch();
  void readModulation();
  void sendCC(uint8_t ccNumber, uint8_t value);
  void sendPitchBend(int16_t value);
  
  uint8_t lastVolume = 0;
  uint16_t lastPitch = 0;
  uint8_t lastModulation = 0;
  unsigned long lastUpdate = 0;
  unsigned long volumeDisplayTime = 0;
  bool volumeDisplayActive = false;
  
  HC595Display* display = nullptr;
  BPMCounter* bpmCounter = nullptr;
};

#endif  // POT_CONTROL_H
