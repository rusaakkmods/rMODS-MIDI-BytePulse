/**
 * MIDI BytePulse - Pro Micro USB MIDI Sync Box
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

uint16_t getCurrentBPM() {
  return sync.getCurrentBPM();
}

void onBPMChanged(uint16_t bpm) {
  display.setBPM(bpm);
}

void onClockStopped() {
  display.clear();
}

void syncInInterrupt() {
  sync.handleSyncInPulse();
}

void processUSBMIDI() {
  while (true) {
    midiEventPacket_t rx = MidiUSB.read();
    
    if (rx.header == 0) break;
    
    if (rx.header == 0x0F) {
      switch (rx.byte1) {
        case 0xF8: 
          sync.handleClock(CLOCK_SOURCE_USB);
          break;
        case 0xFA:
          sync.handleStart(CLOCK_SOURCE_USB);
          break;
        case 0xFB: 
          sync.handleStart(CLOCK_SOURCE_USB);
          break;
        case 0xFC: 
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
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.begin();
  display.clear(); 
  
  sync.begin();
  sync.setDisplay(&display); 
  sync.onBPMUpdate = onBPMChanged; 
  sync.onClockStop = onClockStopped; 
  midiHandler.setSync(&sync);
  midiHandler.setDisplay(&display);
  midiHandler.begin();
  
  attachInterrupt(digitalPinToInterrupt(SYNC_IN_PIN), syncInInterrupt, RISING);
}

void loop() {
  static bool lastButtonState = HIGH;
  static bool buttonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;
  
  bool buttonReading = digitalRead(BUTTON_PIN);
  
  if (buttonReading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonReading != buttonState) {
      buttonState = buttonReading;
      
      if (buttonState == LOW) {
        display.setButtonPressed(true);
        display.showBPM();
      } else {
        display.setButtonPressed(false);
      }
    }
  }
  
  lastButtonState = buttonReading;
  
  midiHandler.update();
  processUSBMIDI();
  sync.update();
  display.flush();
  midiHandler.flushBuffer();
}
