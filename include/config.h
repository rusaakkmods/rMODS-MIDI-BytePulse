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
#define POT_VOLUME_PIN     A0
#define POT_CUTOFF_PIN     A1
#define POT_RESONANCE_PIN  A2

// Rotary Encoder
#define ENCODER_A_PIN       2
#define ENCODER_B_PIN       3
#define ENCODER_BTN_PIN     8

// Buttons
#define BTN_FUNCTION_PIN    9
#define BTN_PLAY_PIN       14
#define BTN_STOP_PIN       15

// LEDs
#define LED_BEAT_PIN       16

// TM1637 7-Segment Display
#define TM1637_CLK_PIN      6
#define TM1637_DIO_PIN     10

#endif  // CONFIG_H
