/**
 * MIDI BytePulse - Potentiometer Control Implementation
 */

#include "PotControl.h"
#include "config.h"
#include <MIDIUSB.h>

#define CC_MODULATION 1
#define CC_VOLUME 7
#define CC_PITCH 74

#define THRESHOLD 2

void PotControl::begin() {
  pinMode(POT_VOLUME_PIN, INPUT);
  pinMode(POT_PITCH_PIN, INPUT);
  pinMode(POT_MODULATION_PIN, INPUT);
  
  lastVolume = analogRead(POT_VOLUME_PIN) >> 3;
  lastPitch = analogRead(POT_PITCH_PIN) >> 3;
  lastModulation = analogRead(POT_MODULATION_PIN) >> 3;
}

void PotControl::update() {
  readPot(POT_VOLUME_PIN, lastVolume, CC_VOLUME);
  readPot(POT_PITCH_PIN, lastPitch, CC_PITCH);
  readPot(POT_MODULATION_PIN, lastModulation, CC_MODULATION);
}

void PotControl::readPot(uint8_t pin, uint8_t& lastValue, uint8_t ccNumber) {
  int reading = analogRead(pin) >> 3;
  
  if (abs(reading - lastValue) > THRESHOLD) {
    lastValue = reading;
    sendCC(ccNumber, lastValue);
  }
}

void PotControl::sendCC(uint8_t ccNumber, uint8_t value) {
  midiEventPacket_t event = {0x0B, 0xB0, ccNumber, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
