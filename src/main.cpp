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
#include "Display.h"

MIDIHandler midiHandler;
Sync sync;
Display display;

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
  // Display disabled due to hardware noise interference with MIDI
  // To use display: add 10Ω resistor + 100nF cap on display VCC
  // Or use separate 5V power supply for display module
  display.begin();  // Animates segments for 2 seconds
  display.clear();  // Clear display after animation
  
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
  
  // Update clock indicator (only updates on start/stop, no impact on timing)
  display.updateClockIndicator(sync.isClockRunning());
}
