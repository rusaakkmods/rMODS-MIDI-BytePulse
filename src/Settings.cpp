/**
 * MIDI BytePulse - Settings Implementation
 */

#include "Settings.h"

Settings::Settings() {
    setDefaults();
}

void Settings::begin() {
    // Try to load settings from EEPROM
    if (!load()) {
        setDefaults();
        save();
    }
}

bool Settings::load() {
    readFromEEPROM();
    return isValid();
}

void Settings::save() {
    _data.magic = EEPROM_MAGIC;
    _data.version = EEPROM_VERSION;
    _data.checksum = calculateChecksum();
    
    writeToEEPROM();
}

void Settings::reset() {
    setDefaults();
    save();
}

void Settings::setPPQN(uint8_t ppqn) {
    if (ppqn < MIN_PPQN) ppqn = MIN_PPQN;
    if (ppqn > MAX_PPQN) ppqn = MAX_PPQN;
    _data.ppqn = ppqn;
}

void Settings::setMidiChannel(uint8_t channel) {
    if (channel < 1) channel = 1;
    if (channel > 16) channel = 16;
    _data.midiChannel = channel;
}

void Settings::setClockSource(ClockSource source) {
    _data.clockSource = (uint8_t)source;
}

bool Settings::isValid() const {
    // Check magic number
    if (_data.magic != EEPROM_MAGIC) {
        return false;
    }
    
    // Check version
    if (_data.version != EEPROM_VERSION) {
        return false;
    }
    
    // Verify checksum
    if (_data.checksum != calculateChecksum()) {
        return false;
    }
    
    // Validate ranges
    if (_data.ppqn < MIN_PPQN || _data.ppqn > MAX_PPQN) {
        return false;
    }
    
    if (_data.midiChannel < 1 || _data.midiChannel > 16) {
        return false;
    }
    
    return true;
}

void Settings::setDefaults() {
    _data.magic = EEPROM_MAGIC;
    _data.version = EEPROM_VERSION;
    _data.ppqn = DEFAULT_PPQN;
    _data.midiChannel = MIDI_CHANNEL;
    _data.clockSource = DEFAULT_CLOCK_SOURCE;
    _data.checksum = calculateChecksum();
}

uint8_t Settings::calculateChecksum() const {
    uint8_t sum = 0;
    
    // Simple XOR checksum over the data (excluding checksum field itself)
    sum ^= (_data.magic >> 8) & 0xFF;
    sum ^= _data.magic & 0xFF;
    sum ^= _data.version;
    sum ^= _data.ppqn;
    sum ^= _data.midiChannel;
    sum ^= _data.clockSource;
    
    return sum;
}

void Settings::writeToEEPROM() {
    uint8_t* dataPtr = (uint8_t*)&_data;
    
    for (size_t i = 0; i < sizeof(SettingsData); i++) {
        EEPROM.update(EEPROM_START_ADDR + i, dataPtr[i]);
    }
}

void Settings::readFromEEPROM() {
    uint8_t* dataPtr = (uint8_t*)&_data;
    
    for (size_t i = 0; i < sizeof(SettingsData); i++) {
        dataPtr[i] = EEPROM.read(EEPROM_START_ADDR + i);
    }
}
