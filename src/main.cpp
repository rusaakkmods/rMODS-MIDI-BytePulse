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

// Interrupt handler for sync input
void syncInInterrupt() {
  sync.handleSyncInPulse();
}

void processUSBMIDI() {
  // Drain entire USB receive buffer to prevent message backup
  while (true) {
    midiEventPacket_t rx = MidiUSB.read();
    
    if (rx.header == 0) break;  // No more messages
    
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
  
  // Sync input interrupt on pin 7 (RISING edge)
  attachInterrupt(digitalPinToInterrupt(SYNC_IN_PIN), syncInInterrupt, RISING);
}

void loop() {
  sync.update();              // Time-critical: Sync output first
  processUSBMIDI();            // Process USB MIDI messages
  midiHandler.update();        // Process DIN MIDI messages  
  MidiUSB.flush();             // Flush USB MIDI buffer
  midiHandler.flushBuffer();   // Flush DIN MIDI buffer
}
