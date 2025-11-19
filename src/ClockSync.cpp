/**
 * MIDI BytePulse - Clock Sync Implementation
 */

#include "ClockSync.h"

ClockSync::ClockSync()
    : _ppqn(DEFAULT_PPQN)
    , _clockDivider(MIDI_CLOCKS_PER_QN / DEFAULT_PPQN)
    , _cableInserted(false)
    , _clockActive(false)
    , _midiClockCounter(0)
    , _pulseCount(0)
    , _pulseStartTime(0)
    , _pulseHigh(false)
    , _beatLedStartTime(0)
    , _beatLedActive(false)
    , _beatClockCounter(0) {
}

void ClockSync::begin() {
    // Configure clock output pin
    pinMode(CLOCK_OUT_PIN, OUTPUT);
    digitalWrite(CLOCK_OUT_PIN, LOW);
    
    // Configure sync cable detection pin with pullup
    pinMode(SYNC_DETECT_PIN, INPUT_PULLUP);
    
    // Configure beat LED pin
    pinMode(LED_BEAT_PIN, OUTPUT);
    digitalWrite(LED_BEAT_PIN, LOW);
    
    updateCableDetection();
    calculateClockDivider();
}

void ClockSync::update() {
    // Update cable detection status
    updateCableDetection();
    
    // Handle pulse timing (end pulse after CLOCK_PULSE_WIDTH ms)
    if (_pulseHigh) {
        unsigned long now = millis();
        if (now - _pulseStartTime >= CLOCK_PULSE_WIDTH) {
            setPulseLow();
        }
    }
    
    // Handle beat LED timing
    if (_beatLedActive) {
        unsigned long now = millis();
        if (now - _beatLedStartTime >= BEAT_LED_DURATION) {
            _beatLedActive = false;
            digitalWrite(LED_BEAT_PIN, LOW);
        }
    }
}

void ClockSync::onMidiClock() {
    if (!_clockActive) return;
    
    // Increment clock counter
    _midiClockCounter++;
    
    // Check if we should generate an output pulse
    if (_midiClockCounter >= _clockDivider) {
        _midiClockCounter = 0;
        
        // Generate pulse (cable detection ignored for now)
        setPulseHigh();
        _pulseCount++;
        
        // Also pulse beat LED at the same rate as sync output for visual confirmation
        _beatLedActive = true;
        _beatLedStartTime = millis();
        digitalWrite(LED_BEAT_PIN, HIGH);
    }
}

void ClockSync::onTransportStart() {
    _clockActive = true;
    _midiClockCounter = 0;
    _beatClockCounter = 0;
    _pulseCount = 0;
}

void ClockSync::onTransportContinue() {
    _clockActive = true;
}

void ClockSync::onTransportStop() {
    _clockActive = false;
    
    // Ensure pulse is low when stopped
    if (_pulseHigh) {
        setPulseLow();
    }
    
    // Turn off beat LED
    _beatLedActive = false;
    digitalWrite(LED_BEAT_PIN, LOW);
}

void ClockSync::reset() {
    _clockActive = false;
    _midiClockCounter = 0;
    _beatClockCounter = 0;
    _pulseCount = 0;
    
    // Ensure outputs are low
    setPulseLow();
    _beatLedActive = false;
    digitalWrite(LED_BEAT_PIN, LOW);
}

void ClockSync::setPPQN(uint8_t ppqn) {
    if (ppqn < MIN_PPQN) ppqn = MIN_PPQN;
    if (ppqn > MAX_PPQN) ppqn = MAX_PPQN;
    
    _ppqn = ppqn;
    calculateClockDivider();
}

void ClockSync::setPulseHigh() {
    digitalWrite(CLOCK_OUT_PIN, HIGH);
    _pulseHigh = true;
    _pulseStartTime = millis();
}

void ClockSync::setPulseLow() {
    digitalWrite(CLOCK_OUT_PIN, LOW);
    _pulseHigh = false;
}

void ClockSync::updateCableDetection() {
    // Read sync detect pin (active HIGH when cable inserted)
    bool detected = digitalRead(SYNC_DETECT_PIN) == HIGH;
    
    // Log state change
    if (detected != _cableInserted) {
        _cableInserted = detected;
    }
}

void ClockSync::calculateClockDivider() {
    // Calculate how many MIDI clocks per output pulse
    // MIDI sends 24 clocks per quarter note
    // We want to output _ppqn pulses per quarter note
    _clockDivider = MIDI_CLOCKS_PER_QN / _ppqn;
}
