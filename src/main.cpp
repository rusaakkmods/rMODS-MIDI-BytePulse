/**
 * MIDI BytePulse - Pro Micro USB MIDI Sync Box
 * Hardware: SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 */

#include <Arduino.h>
#include <MIDIUSB.h>
#include "config.h"
#include "MIDIHandler.h"
#include "SyncOut.h"
#include "TransportControl.h"
#include "PotControl.h"
#include "HC595Display.h"
#include "BPMCounter.h"

MIDIHandler midiHandler;
SyncOut syncOut;
TransportControl transport;
PotControl pots;
HC595Display display(DISPLAY_RCLK_PIN);
BPMCounter bpmCounter;

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
  #if SERIAL_DEBUG
  Serial.begin(DEBUG_BAUD_RATE);
  delay(500);
  DEBUG_PRINTLN("=== MIDI BytePulse v1.0 ===");
  DEBUG_PRINTLN("Pro Micro USB MIDI Sync");
  #endif
  
  syncOut.begin();
  
  #if DISPLAY_ENABLED
  display.begin();
  
  bpmCounter.setDisplay(&display);
  pots.setDisplay(&display);
  transport.setDisplay(&display);
  #endif
  
  bpmCounter.setPotControl(&pots);
  bpmCounter.setTransportControl(&transport);
  pots.setBPMCounter(&bpmCounter);
  transport.setBPMCounter(&bpmCounter);
  syncOut.setBPMCounter(&bpmCounter);
  midiHandler.setSyncOut(&syncOut);
  midiHandler.begin();
  transport.begin();
  pots.begin();
  
  #if SERIAL_DEBUG
  DEBUG_PRINTLN("Ready.");
  #endif
}

void loop() {  
  syncOut.update();
  
  processUSBMIDI();
  midiHandler.update();
  midiHandler.flushBuffer();
  
  // UI updates
  transport.update();
  pots.update();
  
  // Update beat indicator at 100Hz
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= 10) {
    bpmCounter.update();
    lastDisplayUpdate = millis();
  }
  
  // Display multiplexing
  #if DISPLAY_ENABLED
  display.updateDisplay();
  #endif
}
