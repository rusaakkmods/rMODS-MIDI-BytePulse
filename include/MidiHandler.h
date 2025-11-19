/**
 * MIDI BytePulse - MIDI Handler
 * Handles forwarding of MIDI messages from DIN to USB with zero latency
 */

#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <Arduino.h>

class MIDIHandler {
public:
  void begin();
  void update();

private:
  static void forwardDINtoUSB(byte channel, byte type, byte data1, byte data2);
  
  // MIDI callback handlers
  static void handleNoteOn(byte channel, byte note, byte velocity);
  static void handleNoteOff(byte channel, byte note, byte velocity);
  static void handleAfterTouchPoly(byte channel, byte note, byte pressure);
  static void handleControlChange(byte channel, byte controller, byte value);
  static void handleProgramChange(byte channel, byte program);
  static void handleAfterTouchChannel(byte channel, byte pressure);
  static void handlePitchBend(byte channel, int bend);
  static void handleSystemExclusive(byte* data, unsigned size);
  static void handleClock();
  static void handleStart();
  static void handleContinue();
  static void handleStop();
  static void handleActiveSensing();
  static void handleSystemReset();
};

#endif  // MIDI_HANDLER_H
