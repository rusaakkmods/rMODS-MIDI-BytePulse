/**
 * MIDI BytePulse - MIDI Handler Implementation
 * Supports USB MIDI and DIN MIDI with intelligent clock source selection
 */

#include "MidiHandler.h"
#include "ClockSync.h"

// Create DIN MIDI instance
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midiDIN);

// Initialize static instance pointer
MidiHandler* MidiHandler::_instance = nullptr;

MidiHandler::MidiHandler() 
    : _clockSource(DEFAULT_CLOCK_SOURCE)
    , _activeClockSource(CLOCK_AUTO)
    , _lastUSBClockTime(0)
    , _lastDINClockTime(0)
    , _isPlaying(false)
    , _bpm(120)
    , _clockCount(0)
    , _clockSync(nullptr)
    , _lastClockTime(0)
    , _clocksSinceLastBeat(0) {
    _instance = this;
}

void MidiHandler::begin() {
    Serial1.begin(MIDI_BAUD_RATE);
    midiDIN.begin(MIDI_CHANNEL_OMNI);
    
    // Set up clock and transport handlers only (not message forwarding)
    midiDIN.setHandleClock(handleDINClock);
    midiDIN.setHandleStart(handleDINStart);
    midiDIN.setHandleContinue(handleDINContinue);
    midiDIN.setHandleStop(handleDINStop);
    midiDIN.setHandleSystemReset(handleDINSystemReset);
    
    midiDIN.turnThruOff();
    
    DEBUG_PRINTLN("MIDI OK");
}

void MidiHandler::update() {
    // Direct MIDI message reconstruction and forwarding from Serial1 to USB
    static uint8_t statusByte = 0;
    static uint8_t dataByte1 = 0;
    static uint8_t dataCount = 0;
    static uint8_t expectedBytes = 0;
    
    bool needsFlush = false;
    uint8_t messagesProcessed = 0;
    const uint8_t MAX_MESSAGES_PER_UPDATE = 32;  // Process up to 32 messages per call
    
    // Process all available bytes quickly to prevent buffer overflow
    while (Serial1.available() > 0 && messagesProcessed < MAX_MESSAGES_PER_UPDATE) {
        uint8_t b = Serial1.read();
        
        // Status byte detection (bit 7 = 1)
        if (b >= 0x80) {
            statusByte = b;
            dataCount = 0;
            
            uint8_t messageType = statusByte & 0xF0;
            
            // Determine expected data bytes
            if (messageType == 0xF0) {
                // System messages - handle immediately
                if (statusByte == 0xF8 || statusByte == 0xFA || 
                    statusByte == 0xFB || statusByte == 0xFC) {
                    // Clock/Transport - process via MIDI Library for clock sync
                    // These are handled by DIN handlers, just forward to USB
                    MidiUSB.sendMIDI({0x0F, statusByte, 0, 0});
                    needsFlush = true;
                    messagesProcessed++;
                }
                statusByte = 0;
                expectedBytes = 0;
            } else if (messageType == 0xC0 || messageType == 0xD0) {
                expectedBytes = 1;  // Program Change, Channel Pressure
            } else {
                expectedBytes = 2;  // Note On/Off, CC, Pitch Bend, etc
            }
        }
        // Data byte (bit 7 = 0)
        else if (statusByte != 0) {
            if (dataCount == 0) {
                dataByte1 = b;
                dataCount++;
                
                if (expectedBytes == 1) {
                    // 2-byte message complete
                    uint8_t cin = (statusByte & 0xF0) == 0xC0 ? 0x0C : 0x0D;
                    MidiUSB.sendMIDI({cin, statusByte, dataByte1, 0});
                    needsFlush = true;
                    messagesProcessed++;
                    statusByte = 0;
                }
            } else if (dataCount == 1 && expectedBytes == 2) {
                // 3-byte message complete
                uint8_t cin;
                uint8_t msgType = statusByte & 0xF0;
                
                switch (msgType) {
                    case 0x80: cin = 0x08; break;  // Note Off
                    case 0x90: cin = 0x09; break;  // Note On
                    case 0xA0: cin = 0x0A; break;  // Poly Aftertouch
                    case 0xB0: cin = 0x0B; break;  // Control Change
                    case 0xE0: cin = 0x0E; break;  // Pitch Bend
                    default: cin = 0x00; break;
                }
                
                if (cin != 0x00) {
                    MidiUSB.sendMIDI({cin, statusByte, dataByte1, b});
                    needsFlush = true;
                    messagesProcessed++;
                }
                statusByte = 0;
            }
        }
    }
    
    // Single flush after batch processing
    if (needsFlush) {
        MidiUSB.flush();
    }
    
    // Process MIDI Library for clock/transport messages only
    midiDIN.read();
    
    // Process incoming USB MIDI messages
    handleUSBMidi();
    
    // Update active clock source based on activity
    updateActiveClockSource();
}

void MidiHandler::setClockSource(ClockSource source) {
    _clockSource = source;
    DEBUG_PRINTLN("Clk src");
}

void MidiHandler::sendStart() {
    MidiUSB.sendMIDI({0x0F, 0xFA, 0, 0});
    MidiUSB.flush();
    midiDIN.sendRealTime(midi::Start);
    
    _isPlaying = true;
    _clockCount = 0;
    DEBUG_PRINTLN("Start");
}

void MidiHandler::sendContinue() {
    MidiUSB.sendMIDI({0x0F, 0xFB, 0, 0});
    MidiUSB.flush();
    midiDIN.sendRealTime(midi::Continue);
    
    _isPlaying = true;
    DEBUG_PRINTLN("Cont");
}

void MidiHandler::sendStop() {
    MidiUSB.sendMIDI({0x0F, 0xFC, 0, 0});
    MidiUSB.flush();
    midiDIN.sendRealTime(midi::Stop);
    
    _isPlaying = false;
    DEBUG_PRINTLN("Stop");
}

void MidiHandler::sendCC(uint8_t cc, uint8_t value, uint8_t channel) {
    // Send to USB MIDI
    MidiUSB.sendMIDI({
        (uint8_t)(0x0B),           // Control Change
        (uint8_t)(0xB0 | (channel - 1)),  // CC on channel
        cc,
        value
    });
    MidiUSB.flush();
    
    // Send to DIN MIDI (optional forwarding)
    // midiDIN.sendControlChange(cc, value, channel);
}

void MidiHandler::sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Send to USB MIDI
    MidiUSB.sendMIDI({
        (uint8_t)(0x09),           // Note On
        (uint8_t)(0x90 | (channel - 1)),  // Note On on channel
        note,
        velocity
    });
    MidiUSB.flush();
    
    // Send to DIN MIDI (optional forwarding)
    // midiDIN.sendNoteOn(note, velocity, channel);
}

void MidiHandler::sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Send to USB MIDI
    MidiUSB.sendMIDI({
        (uint8_t)(0x08),           // Note Off
        (uint8_t)(0x80 | (channel - 1)),  // Note Off on channel
        note,
        velocity
    });
    MidiUSB.flush();
    
    // Send to DIN MIDI (optional forwarding)
    // midiDIN.sendNoteOff(note, velocity, channel);
}

void MidiHandler::updateBPM() {
    unsigned long now = millis();
    if (_lastClockTime > 0 && _clocksSinceLastBeat >= MIDI_CLOCKS_PER_QN) {
        unsigned long elapsed = now - _lastClockTime;
        if (elapsed > 0) {
            // Calculate BPM: (clocks / elapsed_ms) * ms_per_min / clocks_per_beat
            _bpm = (uint16_t)((60000.0 * MIDI_CLOCKS_PER_QN) / elapsed);
            _clocksSinceLastBeat = 0;
            _lastClockTime = now;
        }
    } else if (_lastClockTime == 0) {
        _lastClockTime = now;
    }
}

// ============================================================================
// USB MIDI Handling
// ============================================================================

void MidiHandler::handleUSBMidi() {
    midiEventPacket_t event;
    
    do {
        event = MidiUSB.read();
        if (event.header != 0) {
            processUSBMidiEvent(event);
        }
    } while (event.header != 0);
}

void MidiHandler::processUSBMidiEvent(midiEventPacket_t event) {
    // Check if this is a clock/transport message
    switch (event.byte1) {
        case 0xF8:  // MIDI Clock
            _lastUSBClockTime = millis();
            
            // Only process if USB is active clock source
            if (_activeClockSource == CLOCK_FORCE_USB || 
                (_activeClockSource == CLOCK_AUTO && _clockSource == CLOCK_AUTO)) {
                
                _clockCount++;
                _clocksSinceLastBeat++;
                updateBPM();
                
                // Forward clock pulse to analog sync
                if (_clockSync && _isPlaying) {
                    _clockSync->onMidiClock();
                }
            }
            break;
            
        case 0xFA:  // Start
            _lastUSBClockTime = millis();
            if (_activeClockSource == CLOCK_FORCE_USB || 
                (_activeClockSource == CLOCK_AUTO && _clockSource == CLOCK_AUTO)) {
                _isPlaying = true;
                _clockCount = 0;
                _lastClockTime = 0;
                _clocksSinceLastBeat = 0;
                
                if (_clockSync) {
                    _clockSync->onTransportStart();
                }
                DEBUG_PRINTLN("USB Start");
            }
            break;
            
        case 0xFB:  // Continue
            _lastUSBClockTime = millis();
            if (_activeClockSource == CLOCK_FORCE_USB || 
                (_activeClockSource == CLOCK_AUTO && _clockSource == CLOCK_AUTO)) {
                _isPlaying = true;
                
                if (_clockSync) {
                    _clockSync->onTransportContinue();
                }
                DEBUG_PRINTLN("USB Cont");
            }
            break;
            
        case 0xFC:  // Stop
            _lastUSBClockTime = millis();
            if (_activeClockSource == CLOCK_FORCE_USB || 
                (_activeClockSource == CLOCK_AUTO && _clockSource == CLOCK_AUTO)) {
                _isPlaying = false;
                
                if (_clockSync) {
                    _clockSync->onTransportStop();
                }
                DEBUG_PRINTLN("USB Stop");
            }
            break;
            
        default:
            // Forward other USB MIDI messages to DIN (optional)
            // This allows the device to act as a USB-to-DIN interface
            break;
    }
}

// ============================================================================
// DIN MIDI Callback Handlers (Static)
// ============================================================================

void MidiHandler::handleDINClock() {
    if (!_instance) return;
    
    _instance->_lastDINClockTime = millis();
    
    // Only process if DIN is active clock source
    if (_instance->_activeClockSource == CLOCK_FORCE_DIN ||
        (_instance->_activeClockSource == CLOCK_AUTO && _instance->_clockSource == CLOCK_AUTO && 
         !_instance->isUSBClockActive())) {
        
        _instance->_clockCount++;
        _instance->_clocksSinceLastBeat++;
        _instance->updateBPM();
        
        // Forward clock pulse to analog sync
        if (_instance->_clockSync && _instance->_isPlaying) {
            _instance->_clockSync->onMidiClock();
        }
    }
}

void MidiHandler::handleDINStart() {
    if (!_instance) return;
    
    _instance->_lastDINClockTime = millis();
    
    if (_instance->_activeClockSource == CLOCK_FORCE_DIN ||
        (_instance->_activeClockSource == CLOCK_AUTO && _instance->_clockSource == CLOCK_AUTO && 
         !_instance->isUSBClockActive())) {
        
        _instance->_isPlaying = true;
        _instance->_clockCount = 0;
        _instance->_lastClockTime = 0;
        _instance->_clocksSinceLastBeat = 0;
        
        if (_instance->_clockSync) {
            _instance->_clockSync->onTransportStart();
        }
        
        DEBUG_PRINTLN("DIN Start");
    }
}

void MidiHandler::handleDINContinue() {
    if (!_instance) return;
    
    _instance->_lastDINClockTime = millis();
    
    if (_instance->_activeClockSource == CLOCK_FORCE_DIN ||
        (_instance->_activeClockSource == CLOCK_AUTO && _instance->_clockSource == CLOCK_AUTO && 
         !_instance->isUSBClockActive())) {
        
        _instance->_isPlaying = true;
        
        if (_instance->_clockSync) {
            _instance->_clockSync->onTransportContinue();
        }
        
        DEBUG_PRINTLN("DIN Cont");
    }
}

void MidiHandler::handleDINStop() {
    if (!_instance) return;
    
    _instance->_lastDINClockTime = millis();
    
    if (_instance->_activeClockSource == CLOCK_FORCE_DIN ||
        (_instance->_activeClockSource == CLOCK_AUTO && _instance->_clockSource == CLOCK_AUTO && 
         !_instance->isUSBClockActive())) {
        
        _instance->_isPlaying = false;
        
        if (_instance->_clockSync) {
            _instance->_clockSync->onTransportStop();
        }
        
        DEBUG_PRINTLN("DIN Stop");
    }
}

void MidiHandler::handleDINSystemReset() {
    if (!_instance) return;
    
    _instance->_isPlaying = false;
    _instance->_clockCount = 0;
    _instance->_lastClockTime = 0;
    _instance->_clocksSinceLastBeat = 0;
    
    if (_instance->_clockSync) {
        _instance->_clockSync->reset();
    }
    
    DEBUG_PRINTLN("Reset");
}

// ============================================================================
// Clock Source Selection Logic
// ============================================================================

void MidiHandler::updateActiveClockSource() {
    switch (_clockSource) {
        case CLOCK_FORCE_USB:
            _activeClockSource = CLOCK_FORCE_USB;
            break;
            
        case CLOCK_FORCE_DIN:
            _activeClockSource = CLOCK_FORCE_DIN;
            break;
            
        case CLOCK_AUTO:
        default:
            if (isUSBClockActive()) {
                if (_activeClockSource != CLOCK_FORCE_USB) {
                    _activeClockSource = CLOCK_FORCE_USB;
                    DEBUG_PRINTLN("->USB");
                }
            } else if (isDINClockActive()) {
                if (_activeClockSource != CLOCK_FORCE_DIN) {
                    _activeClockSource = CLOCK_FORCE_DIN;
                    DEBUG_PRINTLN("->DIN");
                }
            } else {
                _activeClockSource = CLOCK_AUTO;
            }
            break;
    }
}

bool MidiHandler::isUSBClockActive() const {
    unsigned long now = millis();
    return (now - _lastUSBClockTime) < CLOCK_TIMEOUT_MS;
}

bool MidiHandler::isDINClockActive() const {
    unsigned long now = millis();
    return (now - _lastDINClockTime) < CLOCK_TIMEOUT_MS;
}
