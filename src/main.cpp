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
#include "DisplayControl.h"

MIDIHandler midiHandler;
SyncOut syncOut;
TransportControl transport;
PotControl pots;
DisplayControl display;

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
  display.begin();
  syncOut.setDisplayControl(&display);
  midiHandler.setSyncOut(&syncOut);
  midiHandler.begin();
  transport.begin();
  pots.begin();
  
  #if SERIAL_DEBUG
  DEBUG_PRINTLN("Ready.");
  #endif
}

void loop() {
  // MIDI processing has highest priority - do it first and frequently
  processUSBMIDI();
  midiHandler.update();
  
  // Flush MIDI buffer immediately after reading to minimize latency
  midiHandler.flushBuffer();
  
  // Time-critical sync output
  syncOut.update();
  
  // Process more MIDI immediately to prevent buffer overflow
  processUSBMIDI();
  midiHandler.update();
  midiHandler.flushBuffer();
  
  // Lower priority UI updates (only after MIDI is processed)
  transport.update();
  pots.update();
  
  // Update display state indicator (only when transport changes)
  static unsigned long lastDisplayCheck = 0;
  if (millis() - lastDisplayCheck >= 2000) {
    static TransportState lastTransportState = TRANSPORT_STOP;
    TransportState currentState = transport.getState();
    
    // Show "-" when stopped
    if (currentState == TRANSPORT_STOP && lastTransportState != TRANSPORT_STOP) {
      display.showStopIndicator();
    }
    
    lastTransportState = currentState;
    lastDisplayCheck = millis();
  }
}
