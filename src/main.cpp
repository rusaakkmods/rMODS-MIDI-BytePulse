/**
 * MIDI BytePulse - Pro Micro USB MIDI Sync Box
 * 
 * Universal MIDI clock follower/converter with dual clock sources (USB + DIN MIDI).
 * Features USB MIDI, DIN MIDI IN, analog clock sync output, and MIDI CC controls.
 * 
 * Hardware: SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 * See documents/pro_micro_usb_midi_sync_box_breadboard_v_1.md for full specs
 */

#include <Arduino.h>
#include "config.h"
#include "MidiHandler.h"
#include "ClockSync.h"
#include "Controls.h"
#if ENABLE_DISPLAY
#include "Display.h"
#endif
#include "Settings.h"

// ============================================================================
// Global Module Instances
// ============================================================================

MidiHandler midiHandler;
ClockSync clockSync;
Controls controls;
#if ENABLE_DISPLAY
Display display;
#endif
Settings settings;

// ============================================================================
// Setup
// ============================================================================

void setup() {
  #if SERIAL_DEBUG
  Serial.begin(DEBUG_BAUD_RATE);
  delay(1000);  // Wait for serial connection
  DEBUG_PRINTLN("=== MIDI BytePulse v1.0 ===");
  DEBUG_PRINTLN("Universal MIDI Clock Converter");
  #endif
  
  // Initialize settings from EEPROM
  settings.begin();
  
  #if !TEST_MODE
  // Initialize clock sync module
  clockSync.begin();
  clockSync.setPPQN(settings.getPPQN());
  
  // Initialize MIDI handler (USB + DIN) and link with clock sync
  midiHandler.begin();
  midiHandler.setClockSync(&clockSync);
  midiHandler.setClockSource(settings.getClockSource());
  
  // Initialize controls and link with MIDI handler
  controls.begin();
  controls.setMidiHandler(&midiHandler);
  #else
  // Test mode: Initialize MIDI handler (for CC messages) and controls
  midiHandler.begin();
  controls.begin();
  controls.setMidiHandler(&midiHandler);
  #endif
  
  #if ENABLE_DISPLAY
  // Initialize display and link with other modules
  display.begin();
  display.setMidiHandler(&midiHandler);
  display.setClockSync(&clockSync);
  display.setControls(&controls);
  #endif
  
  DEBUG_PRINTLN("System initialized");
  DEBUG_PRINT("Clock source: ");
  ClockSource src = settings.getClockSource();
  DEBUG_PRINTLN(src == CLOCK_AUTO ? "AUTO" : (src == CLOCK_FORCE_USB ? "FORCE_USB" : "FORCE_DIN"));
}

// ============================================================================
// Test Mode Loop
// ============================================================================

#if TEST_MODE
void testModeLoop() {
  // Handle transport buttons
  static bool playWasPressed = false;
  static bool stopWasPressed = false;
  static bool isPlaying = false;  // false = stopped/paused, true = playing
  static bool wasStopped = true;  // true = stopped (not just paused)
  
  // Pan control via encoder (0-127, center at 64)
  static uint8_t panValue = 64;  // Start at center
  static bool encoderButtonWasPressed = false;
  static bool functionButtonWasPressed = false;
  
  // Update controls
  controls.update();
  
  // Handle encoder for pan control (CC10)
  int8_t encoderDelta = controls.getEncoderDelta();
  if (encoderDelta != 0) {
    DEBUG_PRINT("Encoder delta: ");
    DEBUG_PRINTLN(encoderDelta);
    
    int16_t newPan = panValue + encoderDelta;
    
    // Constrain to 0-127 range
    if (newPan < 0) newPan = 0;
    if (newPan > 127) newPan = 127;
    
    panValue = (uint8_t)newPan;
    midiHandler.sendCC(CC_PAN, panValue);
    
    DEBUG_PRINT("Pan value: ");
    DEBUG_PRINTLN(panValue);
    
    // Update display pan value
    display._lastPanValue = panValue;
    strcpy(display._lastControlLabel, "PAN");
  }
  
  // Handle encoder button for Note On/Off test
  bool encoderButtonPressed = controls.encoderPressed();
  if (encoderButtonPressed && !encoderButtonWasPressed) {
    // Button just pressed - send Note On C3
    midiHandler.sendNoteOn(48, 100);  // C3, velocity 100
    DEBUG_PRINTLN("Encoder button pressed - Note On C3");
  } else if (!encoderButtonPressed && encoderButtonWasPressed) {
    // Button just released - send Note Off C3
    midiHandler.sendNoteOff(48, 0);   // C3, velocity 0
    DEBUG_PRINTLN("Encoder button released - Note Off C3");
  }
  encoderButtonWasPressed = encoderButtonPressed;
  
  // Handle function button for display status only
  bool functionButtonPressed = controls.functionPressed();
  if (functionButtonPressed && !functionButtonWasPressed) {
    display._encoderButtonPressed = true;
    DEBUG_PRINTLN("Function button pressed");
  } else if (!functionButtonPressed && functionButtonWasPressed) {
    display._encoderButtonPressed = false;
    DEBUG_PRINTLN("Function button released");
  }
  functionButtonWasPressed = functionButtonPressed;
  
  // Read button states
  bool playPressed = controls.playPressed();
  bool stopPressed = controls.stopPressed();
  
  // Play/Pause button
  if (playPressed && !playWasPressed) {
    if (isPlaying) {
      // Currently playing - pause it
      midiHandler.sendStop();
      isPlaying = false;
      wasStopped = false;  // We're paused, not stopped
      display._wasStopped = false;  // Update display state
    } else {
      // Currently paused or stopped
      if (wasStopped) {
        // Was stopped - send Start (from beginning)
        midiHandler.sendStart();
      } else {
        // Was paused - send Continue (resume)
        midiHandler.sendContinue();
      }
      isPlaying = true;
    }
  }
  playWasPressed = playPressed;
  
  // Stop button
  if (stopPressed && !stopWasPressed) {
    midiHandler.sendStop();
    isPlaying = false;
    wasStopped = true;  // Full stop, reset position
    display._wasStopped = true;  // Update display state
  }
  stopWasPressed = stopPressed;
  
  #if ENABLE_DISPLAY
  display.update();
  #endif
}
#endif

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  #if TEST_MODE
  testModeLoop();
  return;
  #endif
  
  // Handle transport buttons
  static bool playWasPressed = false;
  static bool stopWasPressed = false;
  
  // Read button states
  bool playPressed = controls.playPressed();
  bool stopPressed = controls.stopPressed();
  
  // Update all modules
  midiHandler.update();  // Handles both USB and DIN MIDI
  clockSync.update();
  controls.update();
  #if ENABLE_DISPLAY
  display.update();
  #endif
  
  // Play/Pause button (toggle behavior)
  if (playPressed && !playWasPressed) {
    if (midiHandler.isPlaying()) {
      midiHandler.sendStop();
    } else {
      if (midiHandler.getClockCount() == 0) {
        midiHandler.sendStart();
      } else {
        midiHandler.sendContinue();
      }
    }
  }
  playWasPressed = playPressed;
  
  // Stop button
  if (stopPressed && !stopWasPressed) {
    midiHandler.sendStop();
  }
  stopWasPressed = stopPressed;
  
  // Function button - enter/exit menu
  static bool functionWasPressed = false;
  bool functionPressed = controls.functionPressed();
  
  #if ENABLE_DISPLAY
  if (functionPressed && !functionWasPressed) {
    if (display.isInMenu()) {
      display.exitMenu();
    } else {
      display.enterMenu();
    }
  }
  #endif
  functionWasPressed = functionPressed;
  
  // Handle encoder input
  #if ENABLE_DISPLAY
  int8_t encoderDelta = controls.getEncoderDelta();
  if (encoderDelta != 0 && display.isInMenu()) {
    display.handleEncoderDelta(encoderDelta);
  }
  #endif
  
  // Handle encoder button press
  static bool encoderWasPressed = false;
  bool encoderPressed = controls.encoderPressed();
  
  #if ENABLE_DISPLAY
  if (encoderPressed && !encoderWasPressed) {
    if (display.isInMenu()) {
      display.handleEncoderPress();
      
      // Save settings if changed
      bool settingsChanged = false;
      
      if (clockSync.getPPQN() != settings.getPPQN()) {
        settings.setPPQN(clockSync.getPPQN());
        settingsChanged = true;
      }
      
      if (midiHandler.getClockSource() != settings.getClockSource()) {
        settings.setClockSource(midiHandler.getClockSource());
        settingsChanged = true;
      }
      
      if (settingsChanged) {
        settings.save();
        DEBUG_PRINTLN("Settings saved to EEPROM");
      }
    }
  }
  #endif
  encoderWasPressed = encoderPressed;
}
