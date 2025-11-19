/**
 * MIDI BytePulse - Pro Micro USB MIDI Sync Box
 * Hardware: SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 */

#include <Arduino.h>
#include <MIDIUSB.h>
#include "config.h"
#include "MIDIHandler.h"
#include "SyncOut.h"

MIDIHandler midiHandler;
SyncOut syncOut;

void processUSBMIDI() {
  midiEventPacket_t rx = MidiUSB.read();
  
  if (rx.header == 0) return;
  
  // Handle realtime messages
  if (rx.header == 0x0F) {
    switch (rx.byte1) {
      case 0xF8:  // Clock
        syncOut.handleClock(CLOCK_SOURCE_USB);
        break;
      case 0xFA:  // Start
        syncOut.handleStart(CLOCK_SOURCE_USB);
        break;
      case 0xFB:  // Continue
        syncOut.handleStart(CLOCK_SOURCE_USB);
        break;
      case 0xFC:  // Stop
        syncOut.handleStop(CLOCK_SOURCE_USB);
        break;
    }
  }
}

void setup() {
  syncOut.begin();
  midiHandler.setSyncOut(&syncOut);
  midiHandler.begin();
}

void loop() {
  processUSBMIDI();
  midiHandler.update();
  syncOut.update();
}
