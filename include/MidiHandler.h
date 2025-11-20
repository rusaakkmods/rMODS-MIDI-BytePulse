/**
 * MIDI BytePulse - MIDI Handler
 */

#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <Arduino.h>
#include <MIDIUSB.h>

class Sync;
class Display;

class MIDIHandler {
public:
  void begin();
  void update();
  void setSync(Sync* s);
  void setDisplay(Display* d);
  static void flushBuffer();
  static void forwardUSBtoDIN(const midiEventPacket_t& event);

private:
  static Sync* sync;
  static Display* display;
  
  static void sendMessage(const midiEventPacket_t& event);
  static void forwardDINtoUSB(byte channel, byte type, byte data1, byte data2);
  
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
