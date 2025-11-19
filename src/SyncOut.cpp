/**
 * MIDI BytePulse - Sync Output Implementation
 */

#include "SyncOut.h"
#include "config.h"

#define PULSE_WIDTH_US 5000       // 5ms pulse width for faster response
#define PPQN 24                   // Pulses per quarter note
#define USB_CLOCK_TIMEOUT_MS 100  // If no USB clock for 100ms, switch back to DIN

void SyncOut::begin() {
  pinMode(CLOCK_OUT_PIN, OUTPUT);
  pinMode(LED_BEAT_PIN, OUTPUT);
  digitalWrite(CLOCK_OUT_PIN, LOW);
  digitalWrite(LED_BEAT_PIN, LOW);
  
  ppqnCounter = 0;
  isPlaying = false;
  usbIsPlaying = false;
  clockState = false;
  ledState = false;
  activeSource = CLOCK_SOURCE_NONE;
  lastUSBClockTime = 0;
  prevUSBClockTime = 0;
  avgUSBClockInterval = 0;
}

void SyncOut::handleClock(ClockSource source) {
  // If USB clock received while USB is playing, USB becomes master
  if (source == CLOCK_SOURCE_USB && usbIsPlaying) {
    unsigned long now = millis();
    
    // Calculate interval between clocks for adaptive timeout
    if (prevUSBClockTime > 0) {
      unsigned long interval = now - prevUSBClockTime;
      // Simple moving average (weight new interval more)
      if (avgUSBClockInterval == 0) {
        avgUSBClockInterval = interval;
      } else {
        avgUSBClockInterval = (avgUSBClockInterval * 3 + interval * 5) / 8;
      }
    }
    
    prevUSBClockTime = now;
    lastUSBClockTime = now;
    activeSource = CLOCK_SOURCE_USB;
  }
  
  // If DIN clock received but USB is master, ignore DIN
  if (source == CLOCK_SOURCE_DIN && activeSource == CLOCK_SOURCE_USB) {
    return;
  }
  
  // If DIN clock and USB is not master, DIN is master
  if (source == CLOCK_SOURCE_DIN && activeSource != CLOCK_SOURCE_USB) {
    activeSource = CLOCK_SOURCE_DIN;
  }
  
  if (!isPlaying) return;
  
  // Immediate pulse output - no processing delay
  digitalWrite(CLOCK_OUT_PIN, HIGH);
  clockState = true;
  lastPulseTime = micros();
  
  // Beat LED pulses every quarter note (every 24 clocks)
  if (ppqnCounter == 0) {
    digitalWrite(LED_BEAT_PIN, HIGH);
    ledState = true;
  }
  
  ppqnCounter++;
  if (ppqnCounter >= PPQN) {
    ppqnCounter = 0;
  }
}

void SyncOut::handleStart(ClockSource source) {
  // Track USB playing state and make it master
  if (source == CLOCK_SOURCE_USB) {
    usbIsPlaying = true;
    activeSource = CLOCK_SOURCE_USB;
    lastUSBClockTime = millis();
    prevUSBClockTime = 0;  // Reset for fresh interval calculation
    avgUSBClockInterval = 0;
    isPlaying = true;  // Always start playing when USB starts
    ppqnCounter = 0;
    
    // Immediate outputs
    digitalWrite(CLOCK_OUT_PIN, HIGH);
    digitalWrite(LED_BEAT_PIN, HIGH);
    clockState = true;
    ledState = true;
    lastPulseTime = micros();
    return;
  }
  
  // If USB is playing, ignore DIN start
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;
  }
  
  // If DIN start and USB not playing, DIN is master
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_DIN;
    isPlaying = true;
    ppqnCounter = 0;
    
    // Immediate outputs
    digitalWrite(CLOCK_OUT_PIN, HIGH);
    digitalWrite(LED_BEAT_PIN, HIGH);
    clockState = true;
    ledState = true;
    lastPulseTime = micros();
  }
}

void SyncOut::handleStop(ClockSource source) {
  // Track USB playing state
  if (source == CLOCK_SOURCE_USB) {
    usbIsPlaying = false;
    activeSource = CLOCK_SOURCE_NONE;  // Release control, allow DIN to take over
    avgUSBClockInterval = 0;  // Reset interval tracking
    prevUSBClockTime = 0;
    // Don't stop playing if DIN might still be running - just release USB control
    return;
  }
  
  // If USB is playing, ignore DIN stop
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;
  }
  
  // If DIN stop and USB not playing, process it
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_NONE;
    isPlaying = false;
    ppqnCounter = 0;
    
    // Immediate turn off
    digitalWrite(CLOCK_OUT_PIN, LOW);
    digitalWrite(LED_BEAT_PIN, LOW);
    clockState = false;
    ledState = false;
  }
}

void SyncOut::update() {
  unsigned long currentTime = micros();
  
  // Check for USB timeout
  checkUSBTimeout();
  
  // Turn off clock pulse after pulse width
  if (clockState && (currentTime - lastPulseTime >= PULSE_WIDTH_US)) {
    digitalWrite(CLOCK_OUT_PIN, LOW);
    clockState = false;
  }
  
  // Turn off LED pulse after pulse width
  if (ledState && (currentTime - lastPulseTime >= PULSE_WIDTH_US)) {
    digitalWrite(LED_BEAT_PIN, LOW);
    ledState = false;
  }
}

void SyncOut::checkUSBTimeout() {
  if (!usbIsPlaying || avgUSBClockInterval == 0) return;
  
  unsigned long now = millis();
  unsigned long timeSinceLastClock = now - lastUSBClockTime;
  
  // Timeout = 3x the average interval (handles tempo variations)
  // At 30 BPM: 24 clocks/beat, 30 beats/min = 720 clocks/min = 12 clocks/sec
  // Interval = 1000ms / 12 = 83.3ms per clock
  // Timeout = 83.3 * 3 = 250ms
  // At 400 BPM: interval = 6.25ms, timeout = 18.75ms
  unsigned long timeoutThreshold = avgUSBClockInterval * 3;
  
  if (timeSinceLastClock > timeoutThreshold) {
    // USB has stopped sending clocks without sending stop message
    usbIsPlaying = false;
    activeSource = CLOCK_SOURCE_NONE;
    avgUSBClockInterval = 0;
    prevUSBClockTime = 0;
  }
}

void SyncOut::pulseClock() {
  digitalWrite(CLOCK_OUT_PIN, HIGH);
  clockState = true;
  lastPulseTime = micros();
}

void SyncOut::pulseLED() {
  digitalWrite(LED_BEAT_PIN, HIGH);
  ledState = true;
}
