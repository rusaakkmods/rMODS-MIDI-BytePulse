/**
 * MIDI BytePulse - Display Implementation (U8g2)
 * Lightweight display using U8g2 library
 */

#include "Display.h"
#include "MidiHandler.h"
#include "ClockSync.h"
#include "Controls.h"

Display::Display()
    : _display(U8G2_R0, U8X8_PIN_NONE)
    , _midiHandler(nullptr)
    , _clockSync(nullptr)
    , _controls(nullptr)
    , _mode(MODE_SPLASH)
    , _lastUpdate(0)
    , _splashStartTime(0)
    , _lastActivity(0)
    , _selectedItem(MENU_PPQN)
    , _editingValue(false)
    , _editPPQN(DEFAULT_PPQN)
    , _editChannel(MIDI_CHANNEL)
    , _editClockSource(DEFAULT_CLOCK_SOURCE)
    , _lastVolumeValue(255)
    , _lastCutoffValue(255)
    , _lastResonanceValue(255)
    , _lastPanValue(64)
    , _wasStopped(true)
    , _encoderButtonPressed(false) {
    strcpy(_lastControlLabel, "---");
}

void Display::begin() {
    if (!_display.begin()) {
        DEBUG_PRINTLN("OLED fail");
        return;
    }
    
    showSplash();
    DEBUG_PRINTLN("Disp OK");
}

void Display::update() {
    unsigned long now = millis();
    
    if (now - _lastUpdate < DISPLAY_UPDATE_MS) {
        return;
    }
    _lastUpdate = now;
    
    switch (_mode) {
        case MODE_SPLASH:
            if (now - _splashStartTime >= SPLASH_DURATION_MS) {
                showMain();
            }
            break;
            
        case MODE_MAIN:
            renderMain();
            break;
            
        case MODE_MENU:
            if (now - _lastActivity >= MENU_TIMEOUT_MS) {
                exitMenu();
            } else {
                renderMenu();
            }
            break;
    }
}

void Display::showSplash() {
    _mode = MODE_SPLASH;
    _splashStartTime = millis();
    renderSplash();
}

void Display::showMain() {
    _mode = MODE_MAIN;
    renderMain();
}

void Display::enterMenu() {
    _mode = MODE_MENU;
    _selectedItem = MENU_PPQN;
    _editingValue = false;
    
    if (_clockSync) {
        _editPPQN = _clockSync->getPPQN();
    }
    if (_midiHandler) {
        _editClockSource = _midiHandler->getClockSource();
    }
    _editChannel = MIDI_CHANNEL;
    
    resetMenuTimeout();
    DEBUG_PRINTLN("Menu");
}

void Display::exitMenu() {
    _mode = MODE_MAIN;
    _editingValue = false;
    DEBUG_PRINTLN("Exit");
}

void Display::handleEncoderDelta(int8_t delta) {
    if (_mode != MODE_MENU) return;
    
    resetMenuTimeout();
    
    if (_editingValue) {
        switch (_selectedItem) {
            case MENU_PPQN:
                _editPPQN = constrain(_editPPQN + delta, MIN_PPQN, MAX_PPQN);
                break;
                
            case MENU_CLOCK_SOURCE:
                {
                    int newSource = (int)_editClockSource + delta;
                    if (newSource < 0) newSource = 2;
                    if (newSource > 2) newSource = 0;
                    _editClockSource = (ClockSource)newSource;
                }
                break;
                
            case MENU_MIDI_CHANNEL:
                _editChannel = constrain(_editChannel + delta, 1, 16);
                break;
                
            default:
                break;
        }
    } else {
        int newItem = (int)_selectedItem + delta;
        if (newItem < 0) newItem = MENU_COUNT - 1;
        if (newItem >= MENU_COUNT) newItem = 0;
        _selectedItem = (MenuItem)newItem;
    }
}

void Display::handleEncoderPress() {
    if (_mode != MODE_MENU) return;
    
    resetMenuTimeout();
    
    if (_editingValue) {
        switch (_selectedItem) {
            case MENU_PPQN:
                if (_clockSync) {
                    _clockSync->setPPQN(_editPPQN);
                }
                break;
                
            case MENU_CLOCK_SOURCE:
                if (_midiHandler) {
                    _midiHandler->setClockSource(_editClockSource);
                }
                break;
                
            case MENU_MIDI_CHANNEL:
                break;
                
            default:
                break;
        }
        _editingValue = false;
    } else {
        switch (_selectedItem) {
            case MENU_PPQN:
            case MENU_CLOCK_SOURCE:
            case MENU_MIDI_CHANNEL:
                _editingValue = true;
                break;
                
            case MENU_EXIT:
                exitMenu();
                break;
                
            default:
                break;
        }
    }
}

void Display::renderSplash() {
    _display.firstPage();
    do {
        _display.setFont(u8g2_font_ncenB10_tr);
        _display.drawStr(10, 36, "BytePulse");
    } while (_display.nextPage());
}

void Display::renderMain() {
    char buf[32];
    
    _display.firstPage();
    do {
        _display.setFont(u8g2_font_6x10_tr);
        
        #if TEST_MODE
        // Test mode: Display volume, cutoff, and resonance values (128x32 display)
        _display.setFont(u8g2_font_8x13_tf);
        
        if (_controls) {
            // Get current MIDI values
            uint8_t volumeVal = _controls->getVolumeValue();
            uint8_t cutoffVal = _controls->getCutoffValue();
            uint8_t resonanceVal = _controls->getResonanceValue();
            
            // Check which control changed and update display
            if (volumeVal != _lastVolumeValue) {
                _lastVolumeValue = volumeVal;
                strcpy(_lastControlLabel, "VOL");
            }
            if (cutoffVal != _lastCutoffValue) {
                _lastCutoffValue = cutoffVal;
                strcpy(_lastControlLabel, "CUT");
            }
            if (resonanceVal != _lastResonanceValue) {
                _lastResonanceValue = resonanceVal;
                strcpy(_lastControlLabel, "RES");
            }
            // Pan is updated directly from testModeLoop
            
            // Display the last changed control
            if (strcmp(_lastControlLabel, "VOL") == 0) {
                snprintf(buf, sizeof(buf), "VOL:%d", _lastVolumeValue);
            } else if (strcmp(_lastControlLabel, "CUT") == 0) {
                snprintf(buf, sizeof(buf), "CUT:%d", _lastCutoffValue);
            } else if (strcmp(_lastControlLabel, "RES") == 0) {
                snprintf(buf, sizeof(buf), "RES:%d", _lastResonanceValue);
            } else if (strcmp(_lastControlLabel, "PAN") == 0) {
                snprintf(buf, sizeof(buf), "PAN:%d", _lastPanValue);
            } else {
                snprintf(buf, sizeof(buf), "---");
            }
            _display.drawStr(0, 15, buf);
            
            // Show encoder button status at top right
            if (_encoderButtonPressed) {
                _display.drawStr(80, 15, "PRESSED");
            }
            
            // Show play state at bottom
            if (_midiHandler && _midiHandler->isPlaying()) {
                _display.drawStr(0, 30, "PLAY");
            } else if (_wasStopped) {
                _display.drawStr(0, 30, "STOP");
            } else {
                _display.drawStr(0, 30, "PAUSE");
            }
        } else {
            _display.drawStr(0, 20, "---");
        }
        #else
        // Line 1: Transport status + BPM
        if (_midiHandler && _midiHandler->isPlaying()) {
            snprintf(buf, sizeof(buf), "%dbpm", _midiHandler->getBPM());
            _display.drawStr(0, 10, buf);
            
            ClockSource active = _midiHandler->getActiveClockSource();
            if (active == CLOCK_FORCE_USB) {
                _display.drawStr(60, 10, "USB");
            } else if (active == CLOCK_FORCE_DIN) {
                _display.drawStr(60, 10, "DIN");
            }
        } else {
            _display.drawStr(0, 10, "STOP");
        }
        
        // Cable status
        if (_clockSync && _clockSync->isCableInserted()) {
            _display.drawDisc(120, 7, 3);
        } else {
            _display.drawCircle(120, 7, 3);
        }
        
        // Line 2: Clock info
        if (_clockSync) {
            snprintf(buf, sizeof(buf), "Q%d P%lu", 
                     _clockSync->getPPQN(), 
                     (unsigned long)_clockSync->getPulseCount());
            _display.drawStr(0, 30, buf);
        }
        
        // Line 3: Controls
        if (_controls) {
            snprintf(buf, sizeof(buf), "V%d C%d R%d",
                     _controls->getVolumeValue(),
                     _controls->getCutoffValue(),
                     _controls->getResonanceValue());
            _display.drawStr(0, 50, buf);
        }
        #endif
    } while (_display.nextPage());
}

void Display::renderMenu() {
    char buf[16];
    
    _display.firstPage();
    do {
        _display.setFont(u8g2_font_6x10_tr);
        int y = 12;
        
        // PPQN
        snprintf(buf, sizeof(buf), "%d", _editPPQN);
        if (_selectedItem == MENU_PPQN) _display.drawStr(0, y, ">");
        _display.drawStr(10, y, "PPQN");
        _display.drawStr(70, y, buf);
        if (_selectedItem == MENU_PPQN && _editingValue) {
            _display.drawFrame(68, y-9, 30, 12);
        }
        y += 14;
        
        // Clock Source
        const char* cs = "AUTO";
        if (_editClockSource == CLOCK_FORCE_USB) cs = "USB";
        else if (_editClockSource == CLOCK_FORCE_DIN) cs = "DIN";
        if (_selectedItem == MENU_CLOCK_SOURCE) _display.drawStr(0, y, ">");
        _display.drawStr(10, y, "Clk");
        _display.drawStr(70, y, cs);
        if (_selectedItem == MENU_CLOCK_SOURCE && _editingValue) {
            _display.drawFrame(68, y-9, 30, 12);
        }
        y += 14;
        
        // MIDI Channel
        snprintf(buf, sizeof(buf), "%d", _editChannel);
        if (_selectedItem == MENU_MIDI_CHANNEL) _display.drawStr(0, y, ">");
        _display.drawStr(10, y, "Ch");
        _display.drawStr(70, y, buf);
        if (_selectedItem == MENU_MIDI_CHANNEL && _editingValue) {
            _display.drawFrame(68, y-9, 30, 12);
        }
        y += 18;
        
        // Exit
        if (_selectedItem == MENU_EXIT) _display.drawStr(0, y, ">");
        _display.drawStr(10, y, "Exit");
    } while (_display.nextPage());
}

void Display::resetMenuTimeout() {
    _lastActivity = millis();
}
