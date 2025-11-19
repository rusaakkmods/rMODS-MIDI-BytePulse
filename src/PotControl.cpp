/**
 * MIDI BytePulse - Potentiometer Control Implementation
 */

#include "PotControl.h"
#include "config.h"
#include <MIDIUSB.h>

#define CC_MODULATION 1
#define CC_VOLUME 7

#define THRESHOLD_7BIT 2
#define THRESHOLD_14BIT 8

void PotControl::begin() {
  pinMode(POT_VOL_PIN, INPUT);
  pinMode(POT_MOD1_PIN, INPUT);
  pinMode(POT_MOD2_PIN, INPUT);
  
  lastVolume = analogRead(POT_VOL_PIN) >> 3;
  lastPitch = analogRead(POT_MOD1_PIN);
  lastModulation = analogRead(POT_MOD2_PIN) >> 3;
}

void PotControl::update() {
  readVolume();
  readPitch();
  readModulation();
}

void PotControl::readVolume() {
  int reading = analogRead(POT_VOL_PIN) >> 3;
  
  if (abs(reading - lastVolume) > THRESHOLD_7BIT) {
    lastVolume = reading;
    sendCC(CC_VOLUME, lastVolume);
  }
}

void PotControl::readPitch() {
  int reading = analogRead(POT_MOD1_PIN);
  
  if (abs(reading - (int)lastPitch) > THRESHOLD_14BIT) {
    lastPitch = reading;
    int16_t pitchValue = map(lastPitch, 0, 1023, -8192, 8191);
    sendPitchBend(pitchValue);
  }
}

void PotControl::readModulation() {
  int reading = analogRead(POT_MOD2_PIN) >> 3;
  
  if (abs(reading - lastModulation) > THRESHOLD_7BIT) {
    lastModulation = reading;
    sendCC(CC_MODULATION, lastModulation);
  }
}

void PotControl::sendCC(uint8_t ccNumber, uint8_t value) {
  midiEventPacket_t event = {0x0B, 0xB0, ccNumber, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void PotControl::sendPitchBend(int16_t value) {
  uint16_t bendValue = value + 8192;
  uint8_t lsb = bendValue & 0x7F;
  uint8_t msb = (bendValue >> 7) & 0x7F;
  
  midiEventPacket_t event = {0x0E, 0xE0, lsb, msb};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
