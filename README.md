# MIDI BytePulse — Universal MIDI Clock Converter

**Firmware for Pro Micro USB MIDI Sync Box (v1.0)**

A USB class-compliant MIDI interface and clock converter that bridges the gap between your DAW, hardware synthesizers, and modular gear. Built for the SparkFun Pro Micro (ATmega32U4, 5V, 16MHz).

---

## 🎯 Overview

MIDI BytePulse is a **universal MIDI clock follower/converter** that intelligently routes MIDI clock from either USB (DAW) or DIN MIDI IN (hardware) to generate clean analog sync pulses for vintage gear, Volcas, Teenage Engineering devices, and modular synthesizers.

### Key Features

- **Dual Clock Sources**
  - USB MIDI (from DAW/host computer)
  - DIN MIDI IN (from any hardware sequencer, drum machine, or synth)
  - Intelligent auto-switching with priority control
  
- **Analog Clock Sync Output**
  - 0–5V pulses via protected open-collector transistor
  - Configurable PPQN (1-24, default 2 for Korg/Arturia compatibility)
  - Cable plug detection (only generates pulses when connected)
  
- **MIDI Control Interface**
  - 3 analog pots sending MIDI CC (Volume, Cutoff, Resonance)
  - Transport control buttons (Play/Pause, Stop)
  - Rotary encoder for menu navigation and parameter control
  
- **Visual Feedback**
  - 0.96" OLED display (128×64 SSD1306)
  - Real-time BPM display
  - Active clock source indicator
  - Beat LED (pulses on quarter notes)
  - Power LED
  
- **Configuration**
  - EEPROM persistence (settings survive power cycles)
  - Menu system for clock source, PPQN, and MIDI channel
  - Auto-timeout menu

---

##  Project Structure

```
├── platformio.ini           # PlatformIO configuration & dependencies
├── include/
│   ├── config.h            # Pin definitions, constants, clock source enum
│   ├── MidiHandler.h       # MIDI I/O (USB + DIN), clock routing
│   ├── ClockSync.h         # Analog clock pulse generation
│   ├── Controls.h          # Pots, encoder, buttons with debouncing
│   ├── Display.h           # OLED UI and menu system
│   └── Settings.h          # EEPROM persistence
└── src/
    ├── main.cpp            # Main application logic
    ├── MidiHandler.cpp     # Dual MIDI source implementation
    ├── ClockSync.cpp       # Clock pulse timing & cable detection
    ├── Controls.cpp        # Input handling with noise filtering
    ├── Display.cpp         # UI rendering & menu navigation
    └── Settings.cpp        # EEPROM read/write with validation
```

---

## ⚙️ Clock Source Modes

### 🔄 AUTO (Default)
Automatically selects the active clock source with USB priority.

**Behavior:**
- USB MIDI clock detected → switches to USB
- No USB clock for 3 seconds → switches to DIN MIDI
- No clocks from either source → idle

**Use case:** Flexible studio setup where you switch between DAW and hardware sequencers.

### 🖥️ FORCE USB
Only accepts MIDI clock from USB (ignores DIN MIDI clock).

**Use case:** DAW is always the master, DIN MIDI IN used only for note/CC data.

### 🎹 FORCE DIN
Only accepts MIDI clock from DIN MIDI IN (ignores USB MIDI clock).

**Use case:** External hardware sequencer is the master clock source.

---

## 🎛️ MIDI CC Assignments

| Control      | Pin  | MIDI CC | Description           |
|--------------|------|---------|-----------------------|
| Volume (RV1) | A0   | CC7     | Main Volume           |
| Cutoff (RV2) | A1   | CC74    | Brightness/Filter     |
| Resonance (RV3) | A2 | CC71   | Resonance/Timbre      |

All CC messages are sent via USB MIDI to the host DAW.

---

## 🎮 Controls

### Buttons
- **Function** (D4) - Enter/exit configuration menu
- **Play/Pause** (D5) - Send MIDI Start/Continue/Stop to all outputs
- **Stop** (D7) - Send MIDI Stop to all outputs

### Rotary Encoder
- **Rotate** - Navigate menu items, adjust values
- **Press** - Select menu item, confirm value changes

### Potentiometers
- **Volume, Cutoff, Resonance** - Real-time MIDI CC control
- Noise filtering with 3-count ADC deadzone
- Sends CC only when value actually changes

---

## 📺 Display Modes

### Main Screen
```
PLAY 120 BPM
USB              [●]  ← Cable indicator
PPQN:2  Pulses:48
Vol:64  Cut:82
Res:45
```

Shows:
- Transport state (PLAY/STOP)
- Current BPM
- Active clock source (USB/DIN/---)
- PPQN setting
- Pulse count
- Pot values
- Cable insertion status

### Menu System
```
-- MENU --
> PPQN      [2]
  Clock     AUTO
  MIDI Ch   1
  Exit
```

Items:
- **PPQN** - Clock pulses per quarter note (1-24)
- **Clock** - Source selection (AUTO/USB/DIN)
- **MIDI Ch** - MIDI channel (1-16)
- **Exit** - Return to main screen

Auto-exits after 5 seconds of inactivity.

---

## 🔌 Pin Configuration

See `include/config.h` for complete pin definitions.

**Critical Pins:**
- **D0 (RX1)** - DIN MIDI IN (via 6N138 optocoupler)
- **D6** - Clock sync OUT (via transistor)
- **D16** - Sync cable plug detection
- **A0, A1, A2** - Potentiometers (Volume, Cutoff, Resonance)
- **D2, D3** - Encoder A/B
- **D9** - Encoder button
- **D4, D5, D7** - Function, Play, Stop buttons
- **D8** - Beat LED
- **A4 (SDA), A5 (SCL)** - I2C OLED display

---

## 🚀 Building & Uploading

### Prerequisites
- [PlatformIO](https://platformio.org/) (via VSCode or CLI)
- SparkFun Pro Micro drivers installed

### Build Commands
```bash
# Compile firmware
pio run

# Upload to Pro Micro
pio run --target upload

# Open serial monitor (if DEBUG enabled)
pio device monitor
```

### Debug Mode
Set `SERIAL_DEBUG` to `true` in `include/config.h` to enable serial debugging at 115200 baud.

---

## 🧠 Firmware Architecture

### Modular Design
Each subsystem is isolated in its own class for maintainability:

- **MidiHandler** - Processes USB and DIN MIDI, routes clocks based on source selection
- **ClockSync** - Generates precise analog pulses with cable detection
- **Controls** - Handles all user inputs with debouncing and filtering
- **Display** - Manages OLED rendering and menu state machine
- **Settings** - EEPROM persistence with checksum validation

### Key Design Patterns
- **Singleton pattern** for MIDI callbacks (library requirement)
- **Dependency injection** for inter-module communication
- **State machines** for display modes and clock source selection
- **Debouncing** on all digital inputs (20ms buttons, 5ms encoder)
- **Throttling** for ADC reads (50ms) and display updates (50ms)

### Clock Source Selection Logic
1. User configures mode (AUTO/FORCE_USB/FORCE_DIN) in menu
2. `MidiHandler` tracks last activity time for each source
3. If no clock received for 3 seconds, source considered inactive
4. In AUTO mode, USB has priority; falls back to DIN if USB inactive
5. Only the active source forwards clocks to `ClockSync`

---

## 🔒 Safety Features

- **Open-collector clock output** - Transistor buffer protects MCU from backfeed
- **Cable plug detection** - Only generates pulses when sync cable inserted
- **ADC noise filtering** - 100nF caps + series resistors on pot inputs
- **Input debouncing** - All buttons and encoder properly debounced
- **EEPROM validation** - Magic number + checksum prevents corruption

---

## 📝 Configuration Persistence

Settings stored in EEPROM (survives power cycles):
- PPQN (pulses per quarter note)
- Clock source mode (AUTO/FORCE_USB/FORCE_DIN)
- MIDI channel

**EEPROM Structure:**
```c
struct SettingsData {
    uint16_t magic;        // 0xBEA7 validation
    uint8_t version;       // Structure version
    uint8_t ppqn;
    uint8_t midiChannel;
    uint8_t clockSource;
    uint8_t checksum;      // XOR checksum
};
```

---

## 🎵 Use Cases

### 1. DAW-Centric Studio
**Mode:** AUTO  
**Setup:** DAW → USB → Pro Micro → Analog Clock → Volca/Modular  
**Benefit:** DAW controls everything, hardware stays in sync

### 2. Hardware Sequencer as Master
**Mode:** FORCE_DIN  
**Setup:** Sequencer → DIN MIDI → Pro Micro → Analog Clock → Modular  
**Benefit:** Hardware is master, DAW can still record MIDI

### 3. Hybrid Setup
**Mode:** AUTO  
**Setup:** Both DAW and hardware connected  
**Benefit:** Switch between sources seamlessly, USB takes priority when active

---

## 🐛 Troubleshooting

**No clock pulses:**
- Check cable is inserted (sync detect circuit)
- Verify MIDI clock is being received (watch OLED BPM)
- Check clock source mode matches your setup

**Jittery clock:**
- Ensure PPQN divider is correct (24 MIDI clocks = 1 beat)
- Check for electrical noise near clock output

**Pots sending spurious CC:**
- ADC_DEADZONE set to 3 (can increase in config.h)
- Check for poor connections/cold solder joints

**Settings not saving:**
- EEPROM validation failed - reset to defaults
- Check serial debug for checksum errors

---

## 📚 Libraries Used

- [MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) v5.0.2 - DIN MIDI handling
- [MIDIUSB](https://github.com/arduino-libraries/MIDIUSB) v1.0.5 - USB MIDI for ATmega32U4
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) v2.5.7 - OLED driver
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) v1.11.9 - Graphics primitives
- [Encoder](https://github.com/PaulStoffregen/Encoder) v1.4.4 - Rotary encoder handling

---

## 📄 License

This firmware is provided as-is for personal and educational use.

---

## 🙏 Credits

**Hardware Design:** Based on standard MIDI optocoupler circuits and Pro Micro USB MIDI implementations.

**Author:** rMODS MIDI BytePulse Project

---

## 🔗 Related Documentation

- **Pin Configuration:** `include/config.h` - All pin definitions and constants
- [MIDI Specification](https://www.midi.org/specifications) - Official MIDI protocol docs

---

**Enjoy syncing your gear! 🎹🎛️🎶**
