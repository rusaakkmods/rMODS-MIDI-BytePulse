/**
 * MIDI BytePulse - MIDI Handler
 * Handles MIDI I/O (USB + DIN), clock source selection, and transport control
 */

#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <Arduino.h>
#include <MIDI.h>
#include <MIDIUSB.h>
#include "config.h"

// Forward declarations
class ClockSync;

// External DIN MIDI instance (defined in MidiHandler.cpp)
extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> midiDIN;

class MidiHandler {
public:
    MidiHandler();
    
    void begin();
    void update();
    
    // Clock source configuration
    void setClockSource(ClockSource source);
    ClockSource getClockSource() const { return _clockSource; }
    ClockSource getActiveClockSource() const { return _activeClockSource; }
    
    // Transport Control
    void sendStart();
    void sendContinue();
    void sendStop();
    bool isPlaying() const { return _isPlaying; }
    
    // Note messages (sends to both USB and DIN if available)
    void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel = MIDI_CHANNEL);
    void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel = MIDI_CHANNEL);
    
    // Control Change (sends to both USB and DIN if available)
    void sendCC(uint8_t cc, uint8_t value, uint8_t channel = MIDI_CHANNEL);
    
    // Clock info (calculated from incoming MIDI clock)
    uint16_t getBPM() const { return _bpm; }
    uint32_t getClockCount() const { return _clockCount; }
    
    // Clock sync callback registration
    void setClockSync(ClockSync* clockSync) { _clockSync = clockSync; }
    
private:
    // Clock source management
    ClockSource _clockSource;        // User-configured source
    ClockSource _activeClockSource;  // Currently active source
    unsigned long _lastUSBClockTime;
    unsigned long _lastDINClockTime;
    
    // State
    bool _isPlaying;
    uint16_t _bpm;
    uint32_t _clockCount;
    
    // Clock sync output handler
    ClockSync* _clockSync;
    
    // USB MIDI handling
    void handleUSBMidi();
    void processUSBMidiEvent(midiEventPacket_t event);
    
    // DIN MIDI callback handlers (static for library compatibility)
    static void handleDINClock();
    static void handleDINStart();
    static void handleDINContinue();
    static void handleDINStop();
    static void handleDINSystemReset();
    
    // Clock source selection logic
    void updateActiveClockSource();
    bool isUSBClockActive() const;
    bool isDINClockActive() const;
    
    // BPM calculation
    unsigned long _lastClockTime;
    uint16_t _clocksSinceLastBeat;
    void updateBPM();
    
    // Singleton instance for callbacks
    static MidiHandler* _instance;
};

#endif // MIDI_HANDLER_H
