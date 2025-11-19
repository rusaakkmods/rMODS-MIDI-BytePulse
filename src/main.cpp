/**
 * MIDI BytePulse - Pro Micro USB MIDI Sync Box
 * Hardware: SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 */

#include <Arduino.h>
#include "config.h"
#include "MIDIHandler.h"

MIDIHandler midiHandler;

void setup() {
  midiHandler.begin();
}

void loop() {
  midiHandler.update();
}
