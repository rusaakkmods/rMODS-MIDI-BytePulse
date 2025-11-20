/**
 * MIDI BytePulse - Pro Micro USB MIDI Sync Box
 * Hardware: SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 * Pure MIDI forwarding and clock sync
 */

#include <Arduino.h>
#include <MIDIUSB.h>
#include "config.h"
#include "MIDIHandler.h"
#include "Sync.h"

MIDIHandler midiHandler;
Sync sync;

void syncInInterrupt() {
  sync.handleSyncInPulse();
}

void processUSBMIDI() {
  while (true) {
    midiEventPacket_t rx = MidiUSB.read();
    
    if (rx.header == 0) break;
    
    // Handle realtime messages
    if (rx.header == 0x0F) {
      switch (rx.byte1) {
        case 0xF8:  // Clock
          sync.handleClock(CLOCK_SOURCE_USB);
          break;
        case 0xFA:  // Start
          sync.handleStart(CLOCK_SOURCE_USB);
          break;
        case 0xFB:  // Continue
          sync.handleStart(CLOCK_SOURCE_USB);
          break;
        case 0xFC:  // Stop
          sync.handleStop(CLOCK_SOURCE_USB);
          break;
      }
    }
  }
}

void setup() {
  sync.begin();
  midiHandler.setSync(&sync);
  midiHandler.begin();
  
  attachInterrupt(digitalPinToInterrupt(SYNC_IN_PIN), syncInInterrupt, RISING);
}

void loop() {
  sync.update();
  processUSBMIDI();
  midiHandler.update();
  MidiUSB.flush();
  midiHandler.flushBuffer();
}
