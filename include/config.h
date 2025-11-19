/**
 * MIDI BytePulse - Hardware Configuration
 * Pro Micro Pin Definitions - Minimal Setup
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// MIDI Hardware UART
#define MIDI_IN_PIN         0
#define MIDI_OUT_PIN        1

// Clock Sync Output
#define CLOCK_OUT_PIN       5
#define SYNC_DETECT_PIN     4

// LED
#define LED_BEAT_PIN       10

// Debug
#define SERIAL_DEBUG        false
#define DEBUG_BAUD_RATE    115200

#if SERIAL_DEBUG
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif  // CONFIG_H
