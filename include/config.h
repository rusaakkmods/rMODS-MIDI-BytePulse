/**
 * MIDI BytePulse - Hardware Configuration
 * SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// MIDI Hardware UART
#define MIDI_IN_PIN         0
#define MIDI_OUT_PIN        1

// Clock Sync Output
#define SYNC_OUT_PIN          5
#define SYNC_OUT_DETECT_PIN   4

// Clock Sync Input
#define SYNC_IN_PIN           7
#define SYNC_IN_DETECT_PIN    6

// LED
#define LED_BEAT_PIN       10

// TM1637 Display
#define DISPLAY_CLK_PIN    8
#define DISPLAY_DIO_PIN    9

// Push Button
#define BUTTON_PIN         16

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
