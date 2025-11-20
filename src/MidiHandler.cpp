/**
 * MIDI BytePulse - MIDI Handler Implementation
 */

#include "MIDIHandler.h"
#include "Sync.h"
#include "Display.h"
#include <MIDI.h>
#include <MIDIUSB.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI_DIN);

Sync* MIDIHandler::sync = nullptr;
Display* MIDIHandler::display = nullptr;

void MIDIHandler::sendMessage(const midiEventPacket_t& event) {
  MidiUSB.sendMIDI(event);
}

void MIDIHandler::flushBuffer() {
  MidiUSB.flush();
}

void MIDIHandler::begin() {
  MIDI_DIN.begin(MIDI_CHANNEL_OMNI);
  MIDI_DIN.turnThruOff();
  
  MIDI_DIN.setHandleNoteOn(handleNoteOn);
  MIDI_DIN.setHandleNoteOff(handleNoteOff);
  MIDI_DIN.setHandleAfterTouchPoly(handleAfterTouchPoly);
  MIDI_DIN.setHandleControlChange(handleControlChange);
  MIDI_DIN.setHandleProgramChange(handleProgramChange);
  MIDI_DIN.setHandleAfterTouchChannel(handleAfterTouchChannel);
  MIDI_DIN.setHandlePitchBend(handlePitchBend);
  MIDI_DIN.setHandleSystemExclusive(handleSystemExclusive);
  MIDI_DIN.setHandleClock(handleClock);
  MIDI_DIN.setHandleStart(handleStart);
  MIDI_DIN.setHandleContinue(handleContinue);
  MIDI_DIN.setHandleStop(handleStop);
  MIDI_DIN.setHandleActiveSensing(handleActiveSensing);
  MIDI_DIN.setHandleSystemReset(handleSystemReset);
}

void MIDIHandler::update() {
  MIDI_DIN.read();
}

void MIDIHandler::setSync(Sync* s) {
  sync = s;
}

void MIDIHandler::setDisplay(Display* d) {
  display = d;
}

void MIDIHandler::forwardDINtoUSB(byte channel, byte type, byte data1, byte data2) {
  midiEventPacket_t event;
  event.header = type >> 4;
  event.byte1 = type | (channel - 1);
  event.byte2 = data1;
  event.byte3 = data2;
  
  sendMessage(event);
}

void MIDIHandler::handleNoteOn(byte channel, byte note, byte velocity) {
  forwardDINtoUSB(channel, 0x90, note, velocity);
  if (display && sync && !sync->isClockRunning()) {
    display->showMIDIMessage("n.", note, channel - 1);  // channel is 1-16, convert to 0-15
  }
}

void MIDIHandler::handleNoteOff(byte channel, byte note, byte velocity) {
  forwardDINtoUSB(channel, 0x80, note, velocity);
  // Don't show note off on display
}

void MIDIHandler::handleAfterTouchPoly(byte channel, byte note, byte pressure) {
  forwardDINtoUSB(channel, 0xA0, note, pressure);
}

void MIDIHandler::handleControlChange(byte channel, byte controller, byte value) {
  forwardDINtoUSB(channel, 0xB0, controller, value);
  // Don't show CC on display
}

void MIDIHandler::handleProgramChange(byte channel, byte program) {
  forwardDINtoUSB(channel, 0xC0, program, 0);
  // Don't show PC on display
}

void MIDIHandler::handleAfterTouchChannel(byte channel, byte pressure) {
  forwardDINtoUSB(channel, 0xD0, pressure, 0);
}

void MIDIHandler::handlePitchBend(byte channel, int bend) {
  byte lsb = bend & 0x7F;
  byte msb = (bend >> 7) & 0x7F;
  forwardDINtoUSB(channel, 0xE0, lsb, msb);
}

void MIDIHandler::handleSystemExclusive(byte* data, unsigned size) {
  midiEventPacket_t event;
  event.header = 0x04;
  
  for (unsigned i = 0; i < size; i += 3) {
    event.byte1 = (i < size) ? data[i] : 0;
    event.byte2 = (i + 1 < size) ? data[i + 1] : 0;
    event.byte3 = (i + 2 < size) ? data[i + 2] : 0;
    
    if (i + 3 >= size) {
      if (i + 1 >= size) event.header = 0x05;
      else if (i + 2 >= size) event.header = 0x06;
      else event.header = 0x07;
    }
    
    sendMessage(event);
  }
  MidiUSB.flush();
}

void MIDIHandler::handleClock() {
  midiEventPacket_t event = {0x0F, 0xF8, 0, 0};
  MidiUSB.sendMIDI(event);
  
  static uint8_t clockCounter = 0;
  if (++clockCounter >= 6) {
    clockCounter = 0;
  }
  
  if (sync) {
    sync->handleClock(CLOCK_SOURCE_DIN);
  }
}

void MIDIHandler::handleStart() {
  midiEventPacket_t event = {0x0F, 0xFA, 0, 0};
  MidiUSB.sendMIDI(event);
  
  if (display) {
    display->showPlay();
  }
  
  if (sync) {
    sync->handleStart(CLOCK_SOURCE_DIN);
  }
}

void MIDIHandler::handleContinue() {
  midiEventPacket_t event = {0x0F, 0xFB, 0, 0};
  MidiUSB.sendMIDI(event);
  
  if (display) {
    display->showPlay();
  }
  
  if (sync) {
    sync->handleStart(CLOCK_SOURCE_DIN);
  }
}

void MIDIHandler::handleStop() {
  midiEventPacket_t event = {0x0F, 0xFC, 0, 0};
  MidiUSB.sendMIDI(event);
  
  if (display) {
    display->showStop();
  }
  
  if (sync) {
    sync->handleStop(CLOCK_SOURCE_DIN);
  }
}

void MIDIHandler::handleActiveSensing() {
  midiEventPacket_t event = {0x0F, 0xFE, 0, 0};
  MidiUSB.sendMIDI(event);
}

void MIDIHandler::handleSystemReset() {
  midiEventPacket_t event = {0x0F, 0xFF, 0, 0};
  MidiUSB.sendMIDI(event);
}
