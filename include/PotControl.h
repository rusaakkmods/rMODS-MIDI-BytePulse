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
  void readPot(uint8_t pin, uint8_t& lastValue, uint8_t ccNumber);
  void sendCC(uint8_t ccNumber, uint8_t value);
  
  uint8_t lastVolume = 0;
  uint8_t lastPitch = 0;
  uint8_t lastModulation = 0;
};

#endif  // POT_CONTROL_H
