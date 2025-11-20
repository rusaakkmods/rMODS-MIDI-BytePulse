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

// BPM update callback
void onBPMChanged(uint16_t bpm) {
  display.setBPM(bpm);
}

// Clock stop callback
void onClockStopped() {
  display.clear();
}

// Clock start callback
void onClockStarted() {
  // Just stop idle animation, wait for BPM to be calculated
  // Display will show "PlaY" briefly from the Start message handler
}

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
  #if SERIAL_DEBUG
  Serial.begin(DEBUG_BAUD_RATE);
  while (!Serial && millis() < 3000);  // Wait up to 3 seconds for Serial
  DEBUG_PRINTLN("MIDI BytePulse - Debug Mode");
  DEBUG_PRINTLN("BPM monitoring active (change threshold: >2 BPM)");
  #endif
  
  // Display disabled due to hardware noise interference with MIDI
  // To use display: add 10Ω resistor + 100nF cap on display VCC
  // Or use separate 5V power supply for display module
  display.begin();  // Animates segments for 2 seconds
  display.clear();  // Clear display after animation
  
  sync.begin();
  sync.setDisplay(&display);  // Set display reference for animation sync
  sync.onBPMUpdate = onBPMChanged;  // Set callback for BPM updates
  sync.onClockStop = onClockStopped;  // Set callback for clock stop
  sync.onClockStart = onClockStarted;  // Set callback for clock start
  midiHandler.setSync(&sync);
  midiHandler.setDisplay(&display);
  midiHandler.begin();
  
  attachInterrupt(digitalPinToInterrupt(SYNC_IN_PIN), syncInInterrupt, RISING);
}

void loop() {
  sync.update();
  processUSBMIDI();
  midiHandler.update();
  midiHandler.flushBuffer();  // This already calls MidiUSB.flush()
  display.flush();  // Non-blocking incremental display update
}
