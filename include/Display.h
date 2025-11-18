/**
 * MIDI BytePulse - Display Module
 * Lightweight OLED display using U8g2 library
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "config.h"

// Forward declarations
class MidiHandler;
class ClockSync;
class Controls;

enum DisplayMode {
    MODE_SPLASH,
    MODE_MAIN,
    MODE_MENU
};

enum MenuItem {
    MENU_PPQN,
    MENU_CLOCK_SOURCE,
    MENU_MIDI_CHANNEL,
    MENU_EXIT,
    MENU_COUNT
};

class Display {
public:
    Display();
    
    void begin();
    void update();
    
    void setMidiHandler(MidiHandler* midiHandler) { _midiHandler = midiHandler; }
    void setClockSync(ClockSync* clockSync) { _clockSync = clockSync; }
    void setControls(Controls* controls) { _controls = controls; }
    
    void showSplash();
    void showMain();
    void enterMenu();
    void exitMenu();
    bool isInMenu() const { return _mode == MODE_MENU; }
    
    void handleEncoderDelta(int8_t delta);
    void handleEncoderPress();
    
private:
    U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C _display;
    
    MidiHandler* _midiHandler;
    ClockSync* _clockSync;
    Controls* _controls;
    
    DisplayMode _mode;
    unsigned long _lastUpdate;
    unsigned long _splashStartTime;
    unsigned long _lastActivity;
    
    MenuItem _selectedItem;
    bool _editingValue;
    uint8_t _editPPQN;
    uint8_t _editChannel;
    ClockSource _editClockSource;
    
    // Test mode tracking
    uint8_t _lastVolumeValue;
    uint8_t _lastCutoffValue;
    uint8_t _lastResonanceValue;
    uint8_t _lastPanValue;
    char _lastControlLabel[5];  // "VOL", "CUT", "RES", or "PAN"
    bool _wasStopped;  // Track if fully stopped vs paused
    bool _encoderButtonPressed;  // Track encoder button state
    
    void renderSplash();
    void renderMain();
    void renderMenu();
    void resetMenuTimeout();
    
    friend void loop();  // Allow loop to access _wasStopped
    friend void testModeLoop();  // Allow testModeLoop to access _wasStopped
};

#endif // DISPLAY_H
