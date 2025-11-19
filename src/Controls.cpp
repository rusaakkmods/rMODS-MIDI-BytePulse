/**
 * MIDI BytePulse - Controls Implementation
 */

#include "Controls.h"
#include "MidiHandler.h"

Controls::Controls()
    : _midiHandler(nullptr)
    , _encoder(ENCODER_A_PIN, ENCODER_B_PIN)
    , _lastEncoderPosition(0)
    , _functionPressed(false)
    , _functionLastState(HIGH)
    , _functionLastDebounceTime(0)
    , _playPressed(false)
    , _playLastState(HIGH)
    , _playLastDebounceTime(0)
    , _stopPressed(false)
    , _stopLastState(HIGH)
    , _stopLastDebounceTime(0)
    , _encoderPressed(false)
    , _encoderLastState(HIGH)
    , _encoderLastDebounceTime(0)
    , _volumeValue(0)
    , _cutoffValue(0)
    , _resonanceValue(0)
    , _volumeRaw(0)
    , _cutoffRaw(0)
    , _resonanceRaw(0)
    , _volumeSmooth(0.0f)
    , _cutoffSmooth(0.0f)
    , _resonanceSmooth(0.0f)
    , _lastPotReadTime(0) {
}

void Controls::begin() {
    // Configure button pins with internal pullups
    pinMode(BTN_FUNCTION_PIN, INPUT_PULLUP);
    pinMode(BTN_PLAY_PIN, INPUT_PULLUP);
    pinMode(BTN_STOP_PIN, INPUT_PULLUP);
    pinMode(ENCODER_BTN_PIN, INPUT_PULLUP);
    
    // Configure ADC pins (analog inputs)
    pinMode(POT_VOLUME_PIN, INPUT);
    pinMode(POT_CUTOFF_PIN, INPUT);
    pinMode(POT_RESONANCE_PIN, INPUT);
    
    // Initialize pot values
    _volumeRaw = analogRead(POT_VOLUME_PIN);
    _cutoffRaw = analogRead(POT_CUTOFF_PIN);
    _resonanceRaw = analogRead(POT_RESONANCE_PIN);
    
    _volumeValue = mapAdcToMidi(_volumeRaw);
    _cutoffValue = mapAdcToMidi(_cutoffRaw);
    _resonanceValue = mapAdcToMidi(_resonanceRaw);
}

void Controls::update() {
    updateButtons();
    updatePots();
}

void Controls::updateButtons() {
    // Debounce all buttons
    _functionPressed = debounceButton(BTN_FUNCTION_PIN, _functionLastState, _functionLastDebounceTime);
    _playPressed = debounceButton(BTN_PLAY_PIN, _playLastState, _playLastDebounceTime);
    _stopPressed = debounceButton(BTN_STOP_PIN, _stopLastState, _stopLastDebounceTime);
    _encoderPressed = debounceButton(ENCODER_BTN_PIN, _encoderLastState, _encoderLastDebounceTime);
}

void Controls::updatePots() {
    unsigned long now = millis();
    
    // Throttle pot reading to POT_POLL_INTERVAL
    if (now - _lastPotReadTime < POT_POLL_INTERVAL) {
        return;
    }
    _lastPotReadTime = now;
    
    // Read all pots with oversampling for noise reduction
    uint16_t volumeRaw = oversampleAnalogRead(POT_VOLUME_PIN);
    uint16_t cutoffRaw = oversampleAnalogRead(POT_CUTOFF_PIN);
    uint16_t resonanceRaw = oversampleAnalogRead(POT_RESONANCE_PIN);
    
    // Check for changes and send MIDI CC if changed
    if (potValueChanged(volumeRaw, _volumeRaw, _volumeValue, _volumeSmooth)) {
        if (_midiHandler) {
            _midiHandler->sendCC(CC_VOLUME, _volumeValue);
        }
    }
    
    if (potValueChanged(cutoffRaw, _cutoffRaw, _cutoffValue, _cutoffSmooth)) {
        if (_midiHandler) {
            _midiHandler->sendCC(CC_CUTOFF, _cutoffValue);
        }
    }
    
    if (potValueChanged(resonanceRaw, _resonanceRaw, _resonanceValue, _resonanceSmooth)) {
        if (_midiHandler) {
            _midiHandler->sendCC(CC_RESONANCE, _resonanceValue);
        }
    }
}

int8_t Controls::getEncoderDelta() {
    long newPosition = _encoder.read() / 4;  // Divide by 4: 4 pulses = 1 detent click
    int8_t delta = (int8_t)(newPosition - _lastEncoderPosition);
    _lastEncoderPosition = newPosition;
    
    return delta;
}

bool Controls::debounceButton(uint8_t pin, bool& lastState, unsigned long& lastDebounceTime) {
    bool reading = digitalRead(pin);
    unsigned long now = millis();
    
    // If state changed, reset debounce timer
    if (reading != lastState) {
        lastDebounceTime = now;
        lastState = reading;
    }
    
    // Check if enough time has passed for stable reading
    if ((now - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
        // Button is pressed when pin reads LOW (active low with pullup)
        return (reading == LOW);
    }
    
    return false;
}

uint8_t Controls::mapAdcToMidi(uint16_t adcValue) {
    // Apply calibrated min/max thresholds to ensure full 0-127 range
    // Constrain input to usable range
    if (adcValue <= ADC_MIN_THRESHOLD) {
        return 0;
    }
    if (adcValue >= ADC_MAX_THRESHOLD) {
        return 127;
    }
    
    // Map calibrated range to MIDI (0-127)
    // Scale from (ADC_MIN_THRESHOLD to ADC_MAX_THRESHOLD) -> (0 to 127)
    uint16_t scaledValue = adcValue - ADC_MIN_THRESHOLD;
    uint16_t scaledRange = ADC_MAX_THRESHOLD - ADC_MIN_THRESHOLD;
    
    // Use 32-bit math to avoid overflow
    uint8_t midiValue = (uint8_t)(((uint32_t)scaledValue * 127UL) / scaledRange);
    
    // Safety constrain
    return constrain(midiValue, 0, 127);
}

uint16_t Controls::oversampleAnalogRead(uint8_t pin) {
    // Take multiple readings and use median filter for noise rejection
    // Median filter removes outliers better than averaging
    uint16_t readings[ADC_OVERSAMPLE];
    
    // Collect readings
    for (uint8_t i = 0; i < ADC_OVERSAMPLE; i++) {
        readings[i] = analogRead(pin);
    }
    
    // Simple bubble sort for median
    for (uint8_t i = 0; i < ADC_OVERSAMPLE - 1; i++) {
        for (uint8_t j = 0; j < ADC_OVERSAMPLE - i - 1; j++) {
            if (readings[j] > readings[j + 1]) {
                uint16_t temp = readings[j];
                readings[j] = readings[j + 1];
                readings[j + 1] = temp;
            }
        }
    }
    
    // Return median value (middle of sorted array)
    return readings[ADC_OVERSAMPLE / 2];
}

bool Controls::potValueChanged(uint16_t newRaw, uint16_t& oldRaw, uint8_t& midiValue, float& smoothValue) {
    // Direct conversion - no smoothing for immediate response
    // Hardware filtering (capacitor on pot wiper) recommended for best results
    
    // Convert raw ADC to MIDI value
    uint8_t newMidiValue = mapAdcToMidi(newRaw);
    
    // Only report change if MIDI value changed by at least 3 points
    int8_t midiDiff = abs((int8_t)newMidiValue - (int8_t)midiValue);
    
    if (midiDiff >= 3) {
        oldRaw = newRaw;
        midiValue = newMidiValue;
        return true;
    }
    
    return false;
}
