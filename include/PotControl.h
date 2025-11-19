/**
 * MIDI BytePulse - Potentiometer Control
 */

#ifndef POT_CONTROL_H
#define POT_CONTROL_H

#include <Arduino.h>

class PotControl {
public:
  void begin();
  void update();

private:
  void readVolume();
  void readPitch();
  void readModulation();
  void sendCC(uint8_t ccNumber, uint8_t value);
  void sendPitchBend(int16_t value);
  
  uint8_t lastVolume = 0;
  uint16_t lastPitch = 0;
  uint8_t lastModulation = 0;
};

#endif  // POT_CONTROL_H
