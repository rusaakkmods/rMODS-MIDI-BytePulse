/**
 * MIDI BytePulse - Potentiometer Control Implementation
 */

#include "PotControl.h"
#include "HC595Display.h"
#include "BPMCounter.h"
#include "config.h"
#include <MIDIUSB.h>

#define CC_MODULATION 1
#define CC_VOLUME 7

// Increased thresholds to filter out pot noise
#define THRESHOLD_7BIT 4    // Was 2, now 4 (about 3% change required)
#define THRESHOLD_14BIT 32  // Was 8, now 32 (about 3% change required)
#define POT_UPDATE_INTERVAL 50  // Only read pots every 50ms
#define VOLUME_DISPLAY_DURATION 2000  // Show volume for 2 seconds

void PotControl::begin() {
  pinMode(POT_VOL_PIN, INPUT);
  pinMode(POT_MOD1_PIN, INPUT);
  pinMode(POT_MOD2_PIN, INPUT);
  
  lastVolume = analogRead(POT_VOL_PIN) >> 3;
  lastPitch = analogRead(POT_MOD1_PIN);
  lastModulation = analogRead(POT_MOD2_PIN) >> 3;
}

void PotControl::update() {
  // Throttle pot reading to reduce noise-induced MIDI spam
  unsigned long now = millis();
  if (now - lastUpdate < POT_UPDATE_INTERVAL) {
    return;
  }
  lastUpdate = now;
  
  readVolume();
  readPitch();
  readModulation();
  
  // Check if volume display timeout expired - restore BPM display
  if (volumeDisplayActive && display && bpmCounter) {
    if (now - volumeDisplayTime >= VOLUME_DISPLAY_DURATION) {
      volumeDisplayActive = false;
      // Restore appropriate display based on play state
      if (bpmCounter->getBPM() > 0) {
        display->showBPM(bpmCounter->getBPM());
      } else {
        // Restore stopped state display
        display->showStopped();
        display->setDecimalPoint(3, true);  // Show decimal when stopped
      }
    }
  }
}

void PotControl::readVolume() {
  int reading = analogRead(POT_VOL_PIN) >> 3;
  
  if (abs(reading - lastVolume) > THRESHOLD_7BIT) {
    lastVolume = reading;
    sendCC(CC_VOLUME, lastVolume);
    
    // Show volume on display for 2 seconds
    if (display) {
      display->showVolume(lastVolume);
      volumeDisplayActive = true;
      volumeDisplayTime = millis();
      
      // Set decimal based on play state
      if (bpmCounter && bpmCounter->isPlaying()) {
        // Blinking will be handled by beat indicator in bpmCounter
      } else {
        // Turn off decimal when stopped
        display->setDecimalPoint(3, false);
      }
    }
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
  // Don't flush - let main loop batch them
}

void PotControl::sendPitchBend(int16_t value) {
  uint16_t bendValue = value + 8192;
  uint8_t lsb = bendValue & 0x7F;
  uint8_t msb = (bendValue >> 7) & 0x7F;
  
  midiEventPacket_t event = {0x0E, 0xE0, lsb, msb};
  MidiUSB.sendMIDI(event);
  // Don't flush - let main loop batch them
}
