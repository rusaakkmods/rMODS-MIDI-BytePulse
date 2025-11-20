# rMODS MIDI BytePulse

**Universal MIDI Clock Sync Box** - Multi-source MIDI clock synchronization with analog sync I/O, BPM display, and intelligent source switching.

[![Platform](https://img.shields.io/badge/platform-ATmega32U4-blue.svg)](https://www.sparkfun.com/products/12640)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

## 🎵 Features

### Multi-Source Clock Support
- **USB MIDI Clock** - Computer/DAW sync via native USB MIDI
- **DIN MIDI Clock** - Hardware MIDI IN port (5-pin DIN)
- **Analog Sync Input** - Compatible with modular/eurorack gear (3.5mm jack)
- **Intelligent Source Switching** - Automatically selects active clock source with priority management

### Clock Distribution
- **USB MIDI Clock Output** - Send sync to DAW/software
- **DIN MIDI Clock Output** - Hardware MIDI OUT (5-pin DIN)
- **Analog Sync Output** - Trigger output for modular/analog gear (3.5mm jack)
- **Cable Detection** - Automatically enables/disables outputs based on connected cables

### Display & Monitoring
- **4-Digit 7-Segment Display** (TM1637)
- **Real-time BPM Calculation** - Accurate tempo detection from any source
- **Clock Animation** - Rotating pattern shows clock activity
- **Beat Position Indicator** - Decimal points show quarter note positions (1-4)
- **Push Button BPM Display** - Hold button to view current tempo ("t.###")
- **Idle Display** - Shows "IdLE" when no clock is detected

### MIDI Message Handling
- **Full MIDI Passthrough** - All MIDI messages forwarded between USB ↔ DIN
- **Standard Clock Messages** - Start (0xFA), Stop (0xFC), Continue (0xFB), Clock (0xF8)
- **Active Sensing** - Automatic timeout detection for USB sources
- **No Latency** - Optimized for real-time performance with zero blocking delays

---

## 🔧 Hardware

### Platform
- **SparkFun Pro Micro** (ATmega32U4, 5V, 16MHz)
- Native USB MIDI support (no FTDI required)
- 28KB Flash / 2.5KB RAM

### Pin Configuration

| Pin | Function | Description |
|-----|----------|-------------|
| 0 | MIDI IN | Hardware Serial RX (5-pin DIN) |
| 1 | MIDI OUT | Hardware Serial TX (5-pin DIN) |
| 5 | SYNC OUT | Analog clock output (3.5mm jack) |
| 4 | SYNC OUT DETECT | Cable detection for sync output |
| 7 | SYNC IN | Analog clock input (3.5mm jack) |
| 6 | SYNC IN DETECT | Cable detection for sync input |
| 8 | DISPLAY CLK | TM1637 clock line |
| 9 | DISPLAY DIO | TM1637 data line |
| 10 | LED (future) | Beat indicator LED |
| 16 | BUTTON | BPM display button (INPUT_PULLUP) |

### Connections

**MIDI (5-pin DIN):**
- Standard MIDI circuit with 220Ω resistors and 6N138 optocoupler
- IN: Pin 0 (RX1) via optocoupler
- OUT: Pin 1 (TX1) via 220Ω resistor

**Analog Sync (3.5mm mono jacks):**
- INPUT: Pin 7 (interrupt-capable), 5V trigger signal
- OUTPUT: Pin 5 (direct digital output), 5V trigger pulses
- Cable detection via switched jacks to pins 4 & 6

**Display (TM1637 4-digit):**
- CLK: Pin 8
- DIO: Pin 9
- VCC: 5V
- GND: GND

**Button:**
- One side to Pin 16
- Other side to GND
- Internal pullup enabled (no external resistor needed)

---

## ⚙️ Technical Specifications

### Timing & Synchronization
- **PPQN Resolution:** 24 PPQN (Pulses Per Quarter Note) - MIDI standard
- **BPM Range:** 30-300 BPM (auto-calculated)
- **Clock Accuracy:** Microsecond-precision interrupt handling
- **Latency:** <1ms typical (non-blocking architecture)

### Clock Source Priority
1. **Sync Input** - Highest priority (modular/analog gear)
2. **DIN MIDI** - Hardware MIDI IN port
3. **USB MIDI** - Computer/DAW (with 3-second timeout)

When multiple sources are active, the device automatically switches to the highest priority source.

### Memory Usage
- **Flash:** ~16.9 KB / 28 KB (59%)
- **RAM:** ~1.5 KB / 2.5 KB (59%)
- **Build Optimizations:** LTO, function/data sections, relaxed linking

---

## 📋 Usage

### Basic Operation

**Power On:**
- Display shows "IdLE" when no clock is detected
- Device automatically detects connected cables

**Clock Playback:**
1. Connect a clock source (USB MIDI from DAW, DIN MIDI, or analog sync)
2. Start playback from your source device
3. Display shows rotating animation synchronized to clock
4. Decimal points indicate beat positions (1.2.3.4.)
5. All connected outputs receive synchronized clock

**View BPM:**
- Press and hold the button
- Display shows "t.###" (e.g., "t.120" for 120 BPM)
- Release button to return to normal display
- If no clock detected, shows "IdLE" while held

**Clock Stopping:**
- Stop playback on source device
- Display clears and returns to "IdLE"
- All outputs stop sending clock

### Connection Scenarios

**Scenario 1: DAW → MIDI BytePulse → Hardware Synth**
```
Computer (USB MIDI) → BytePulse → DIN MIDI OUT → Synth
```
- Synth receives standard 24 PPQN MIDI clock
- BPM adjustments in DAW reflected immediately
- All MIDI notes/CC messages pass through

**Scenario 2: Beatstep Pro → MIDI BytePulse → DAW**
```
Beatstep Pro (clock out) → Sync IN → BytePulse → USB MIDI → DAW
```
- Beatstep tempo controls DAW
- DAW locks to external hardware tempo
- Analog clock converted to standard MIDI clock (24 PPQN)

**Scenario 3: Modular → MIDI BytePulse → Multiple Destinations**
```
Eurorack Clock → Sync IN → BytePulse → USB MIDI + DIN MIDI + Sync OUT
```
- Single modular clock source drives everything
- MIDI devices receive standard clock
- Additional modular gear gets buffered sync signal

**Scenario 4: DAW → MIDI BytePulse → Modular**
```
DAW (USB MIDI) → BytePulse → Sync OUT → Eurorack
```
- Software tempo controls modular setup
- 24 PPQN MIDI clock converted to trigger pulses
- Modular oscillators/sequencers sync to DAW

### MIDI Implementation

**Transmitted Messages:**
- `0xF8` - Clock (24 per quarter note)
- `0xFA` - Start
- `0xFC` - Stop
- `0xFB` - Continue
- `0xFE` - Active Sensing (USB only)
- All note/CC/program change messages (passthrough)

**Received Messages:**
- All standard MIDI messages received and processed
- Clock messages trigger sync engine
- Non-clock messages forwarded between USB ↔ DIN

---

## 🛠️ Building & Flashing

### Prerequisites
- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- SparkFun Pro Micro board definitions
- USB cable (micro-USB)

### Dependencies
All dependencies auto-installed via PlatformIO:
```ini
- MIDI Library v5.0.2 (fortyseveneffects)
- MIDIUSB v1.0.5 (Arduino)
- AceSegment v0.13.0 (Brian Park)
```

### Build & Upload

**Using PlatformIO:**
```bash
# Build project
pio run

# Upload to device
pio run --target upload

# Monitor serial output (debug mode)
pio device monitor
```

**Using PlatformIO IDE (VS Code):**
1. Open project folder in VS Code
2. Click "Build" (✓) in status bar
3. Click "Upload" (→) in status bar

### Debug Mode
Enable serial debugging in `config.h`:
```cpp
#define SERIAL_DEBUG    true
#define DEBUG_BAUD_RATE 115200
```
- Prints BPM changes to Serial Monitor
- Threshold: >2 BPM change for logging
- Auto-timeout: Waits 3 seconds for serial connection

---

## 🎛️ Configuration

### Adjustable Parameters

**`config.h` - Hardware Pins:**
```cpp
#define SYNC_IN_PIN         7    // Analog sync input
#define SYNC_OUT_PIN        5    // Analog sync output
#define BUTTON_PIN         16    // BPM display button
#define DISPLAY_CLK_PIN     8    // TM1637 clock
#define DISPLAY_DIO_PIN     9    // TM1637 data
```

**`Sync.cpp` - Timing Constants:**
```cpp
static const unsigned long USB_TIMEOUT = 3000;           // USB inactivity timeout (ms)
static const unsigned long CLOCK_TIMEOUT = 2000;         // Clock source timeout (ms)
static const unsigned long MIN_CLOCK_INTERVAL = 8333;    // ~30 BPM limit (μs)
static const unsigned long MAX_CLOCK_INTERVAL = 50000;   // ~300 BPM limit (μs)
```

**`Display.cpp` - Display Behavior:**
```cpp
static const unsigned long MIDI_MESSAGE_DURATION = 300;  // Message display time (ms)
static const unsigned long IDLE_ANIM_INTERVAL = 300;     // Idle animation speed (ms)
```

---

## 📖 Code Architecture

### Main Components

**`main.cpp`** - Application entry point
- Setup: Initializes all subsystems
- Loop: Polls button, processes USB MIDI, updates sync and display
- Interrupt: Handles sync input pulses (ISR)

**`Sync.cpp/h`** - Clock synchronization engine
- Multi-source clock management with priority
- BPM calculation (240,000 / interval for 4-beat measure)
- Clock distribution to all outputs
- Cable detection logic

**`Display.cpp/h`** - TM1637 display controller
- Non-blocking display updates (AceSegment library)
- Clock-synced animations (16-step rotation)
- Beat position indicators (decimal points)
- BPM display mode

**`MIDIHandler.cpp/h`** - MIDI I/O management
- USB ↔ DIN MIDI passthrough
- Message parsing and forwarding
- Optimized buffer flushing

**`config.h`** - Hardware configuration
- Pin definitions
- Debug settings
- Compile-time constants

### Data Flow

```
┌─────────────┐
│  MIDI IN    │ ──┐
│ (USB/DIN)   │   │
└─────────────┘   │
                  ├──→ MIDIHandler ──→ Sync Engine ──┬──→ USB MIDI OUT
┌─────────────┐   │                                   │
│  SYNC IN    │ ──┘                                   ├──→ DIN MIDI OUT
│ (Interrupt) │                                       │
└─────────────┘                                       └──→ SYNC OUT
       │
       └──────────────────→ Display
```

---

## 🔍 Troubleshooting

### Display shows "IdLE" constantly
- **No clock detected** - Check MIDI cables and ensure source device is playing
- **Wrong cable type** - Use standard MIDI cables (not null-modem)
- **USB not recognized** - Try different USB port or cable

### BPM display shows wrong tempo
- **Source switching** - Device may be receiving clock from unexpected source
- **Sync input PPQN** - Analog sources may use non-standard resolutions (see below)
- **Clock dropouts** - Check cable connections and signal integrity

### Analog sync not working
- **Voltage levels** - Ensure sync source outputs 5V triggers (or use level shifter)
- **Cable detection** - Use switched jacks or modify detection circuit
- **Interrupt conflicts** - Pin 7 must be interrupt-capable (don't change)

### MIDI messages not passing through
- **Baud rate** - Hardware MIDI must be 31250 baud (set in MIDI Library)
- **Optocoupler** - Check 6N138 wiring and power supply
- **USB driver** - Update USB MIDI drivers on computer

### High memory usage / crashes
- **Buffer overflow** - Reduce SERIAL_RX/TX_BUFFER_SIZE in platformio.ini
- **Display updates** - Ensure `display.flush()` called regularly
- **Interrupt safety** - Don't add Serial.print() in ISR functions

---

## 📐 PPQN & Analog Sync

### Understanding PPQN

**PPQN** = Pulses Per Quarter Note (clock resolution)

- **MIDI Standard:** 24 PPQN (fixed)
- **Analog Devices:** Varies widely (1, 2, 4, 8, 24, 48 PPQN)

This device **always outputs 24 PPQN** on MIDI outputs (standard).

### Analog Sync Input

When receiving analog sync (e.g., from Beatstep Pro, Volca, modular), the device **assumes 24 PPQN** and calculates BPM accordingly:

**Example:**
- Beatstep Pro outputs **1 PPQN** (1 pulse per quarter note)
- Device receives 120 pulses/minute
- BPM calculation: `(120 pulses/min × 24) / 24 = 120 BPM` ✓

**Common Analog Sync Resolutions:**
- **Korg Volca Series:** 2 PPQN (1 pulse per 8th note)
- **DIN Sync (vintage):** 24 PPQN (Roland TR-series)
- **Eurorack Clocks:** Often 4, 8, or 24 PPQN
- **Arturia Beatstep Pro:** 1 PPQN (1 pulse per quarter note)

> **Note:** If BPM display seems incorrect, your analog source may be using non-standard PPQN. The clock will still sync correctly, but BPM reading will be scaled.

---

## 🚀 Future Enhancements

Potential features for future versions:
- [ ] EEPROM settings persistence
- [ ] Swing/groove quantization
- [ ] Tap tempo button
- [ ] PPQN configuration menu
- [ ] MIDI message filtering
- [ ] Multiple sync output modes
- [ ] Adjustable LED brightness
- [ ] Clock divider/multiplier

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Credits

**Libraries:**
- [MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) by FortySevenEffects
- [MIDIUSB](https://github.com/arduino-libraries/MIDIUSB) by Arduino
- [AceSegment](https://github.com/bxparks/AceSegment) by Brian Park

**Hardware:**
- SparkFun Pro Micro board design

---
