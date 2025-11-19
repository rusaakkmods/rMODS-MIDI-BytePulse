/**
 * MIDI BytePulse - Hardware Configuration
 * Pro Micro Pin Definitions
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

// Analog Inputs (Potentiometers)
#define POT_VOL_PIN         A0
#define POT_MOD1_PIN        A1
#define POT_MOD2_PIN        A2

// Rotary Encoder
#define ENCODER_A_PIN       2
#define ENCODER_B_PIN       3
#define ENCODER_BTN_PIN     8

// Buttons
#define BTN_FUNCTION_PIN    9
#define BTN_PLAY_PIN        6
#define BTN_STOP_PIN        7

// LEDs
#define LED_BEAT_PIN       10

// 74HC595 7-Segment Display Module (Dual Shift Register)
// Hardware SPI + Latch for non-blocking updates
#define DISPLAY_DIO_PIN    16  // MOSI - Hardware SPI Data
#define DISPLAY_SCLK_PIN   15  // SCK  - Hardware SPI Clock
#define DISPLAY_RCLK_PIN   14  // Latch - Register Clock (any GPIO)

// Display Configuration
#define DISPLAY_ENABLED     false  // Set to false to disable display
#define POTS_ENABLED        false  // Set to false to disable pot controls (max stability)


#define SERIAL_DEBUG        false
#define DEBUG_BAUD_RATE    115200 // Debug serial baud rate

#if SERIAL_DEBUG
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif  // CONFIG_H
