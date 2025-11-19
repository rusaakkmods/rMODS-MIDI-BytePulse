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

MidiHandler midiHandler;

void setup() {
  midiHandler.begin();
}

void loop() {
  midiHandler.update();
}
