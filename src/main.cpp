/**
 * MIDI BytePulse - Pro Micro USB MIDI Sync Box
 * Hardware: SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 * Pure MIDI forwarding and clock sync
 */

#include <Arduino.h>
#include <MIDIUSB.h>
#include "config.h"
#include "MIDIHandler.h"
#include "SyncOut.h"

MIDIHandler midiHandler;
SyncOut syncOut;

void processUSBMIDI() {
  // Drain entire USB receive buffer to prevent message backup
  while (true) {
    midiEventPacket_t rx = MidiUSB.read();
    
    if (rx.header == 0) break;  // No more messages
    
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
}

void setup() {
  #if SERIAL_DEBUG
  Serial.begin(DEBUG_BAUD_RATE);
  delay(500);
  DEBUG_PRINTLN("=== MIDI BytePulse v1.0 ===");
  DEBUG_PRINTLN("Pure MIDI Sync Box");
  #endif
  
  syncOut.begin();
  midiHandler.setSyncOut(&syncOut);
  midiHandler.begin();
  
  #if SERIAL_DEBUG
  DEBUG_PRINTLN("Ready.");
  #endif
}

void loop() {  
  // Time-critical: Sync output first
  syncOut.update();
  
  // MIDI processing
  processUSBMIDI();
  midiHandler.update();
  
  // Flush batched messages
  midiHandler.flushBuffer();
}
