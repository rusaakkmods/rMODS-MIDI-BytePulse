/**
 * MIDI BytePulse - MIDI Handler
 */

#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <Arduino.h>
#include <MIDIUSB.h>

#define MIDI_BUFFER_SIZE 64

class SyncOut;

class MIDIHandler {
public:
  void begin();
  void update();
  void setSyncOut(SyncOut* sync);
  void flushBuffer();

private:
  static SyncOut* syncOut;
  
  // Ring buffer for MIDI messages
  static midiEventPacket_t midiBuffer[MIDI_BUFFER_SIZE];
  static volatile uint8_t bufferHead;
  static volatile uint8_t bufferTail;
  
  static void bufferMessage(const midiEventPacket_t& event);
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
