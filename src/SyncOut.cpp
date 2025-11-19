/**
 * MIDI BytePulse - Sync Output Implementation
 */

#include "SyncOut.h"
#include "config.h"

#define PULSE_WIDTH_US 5000
#define PPQN 24
#define CLOCK_OUT_PPQN 24  // Output 24 PPQN (MIDI clock standard - universal compatibility)

void SyncOut::begin() {
  pinMode(CLOCK_OUT_PIN, OUTPUT);
  pinMode(SYNC_DETECT_PIN, INPUT_PULLUP);
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
  lastDINClockTime = 0;
  prevDINClockTime = 0;
  avgDINClockInterval = 0;
}

void SyncOut::handleClock(ClockSource source) {
  unsigned long now = millis();
  
  if (source == CLOCK_SOURCE_USB && usbIsPlaying) {
    if (prevUSBClockTime > 0) {
      unsigned long interval = now - prevUSBClockTime;
      
      // Light smoothing for timeout calculation only
      if (avgUSBClockInterval == 0) {
        avgUSBClockInterval = interval;
      } else {
        avgUSBClockInterval = (avgUSBClockInterval * 3 + interval) / 4;
      }
    }
    
    prevUSBClockTime = now;
    lastUSBClockTime = now;
    activeSource = CLOCK_SOURCE_USB;
  }
  
  if (source == CLOCK_SOURCE_DIN && activeSource != CLOCK_SOURCE_USB) {
    if (prevDINClockTime > 0) {
      unsigned long interval = now - prevDINClockTime;
      
      // Light smoothing for timeout calculation only
      if (avgDINClockInterval == 0) {
        avgDINClockInterval = interval;
      } else {
        avgDINClockInterval = (avgDINClockInterval * 3 + interval) / 4;
      }
    }
    
    prevDINClockTime = now;
    lastDINClockTime = now;
    activeSource = CLOCK_SOURCE_DIN;
  }
  
  if (source == CLOCK_SOURCE_USB && !usbIsPlaying) {
    usbIsPlaying = true;
    isPlaying = true;
    activeSource = CLOCK_SOURCE_USB;
    ppqnCounter = 0;
    lastUSBClockTime = millis();
    prevUSBClockTime = 0;
    avgUSBClockInterval = 0;
  }
  
  if (source == CLOCK_SOURCE_DIN && activeSource == CLOCK_SOURCE_USB) {
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && activeSource != CLOCK_SOURCE_USB) {
    if (!isPlaying) {
      isPlaying = true;
      activeSource = CLOCK_SOURCE_DIN;
      ppqnCounter = 0;  // Start at 0 for proper first beat
      lastDINClockTime = millis();
      prevDINClockTime = 0;
      avgDINClockInterval = 0;
    }
  }
  
  if (!isPlaying) return;
  
  // Output clock pulse at 2 PPQN (every 12 MIDI clocks)
  // PPQN 24 / OUTPUT 2 = divider 12
  if (ppqnCounter % (PPQN / CLOCK_OUT_PPQN) == 0) {
    if (isJackConnected()) {
      digitalWrite(CLOCK_OUT_PIN, HIGH);
      clockState = true;
      lastPulseTime = micros();
    }
  }
  
  // Handle beat LED on downbeat (ppqnCounter == 0)
  if (ppqnCounter == 0) {
    digitalWrite(LED_BEAT_PIN, HIGH);
    ledState = true;
    if (!clockState) {
      lastPulseTime = micros();  // Sync LED timing with clock if not already pulsing
    }
  }
  
  // Increment counter after processing current beat
  ppqnCounter++;
  if (ppqnCounter >= PPQN) {
    ppqnCounter = 0;
  }
}

void SyncOut::handleStart(ClockSource source) {
  if (source == CLOCK_SOURCE_USB) {
    usbIsPlaying = true;
    activeSource = CLOCK_SOURCE_USB;
    lastUSBClockTime = millis();
    prevUSBClockTime = 0;
    avgUSBClockInterval = 0;
    isPlaying = true;
    ppqnCounter = 0;  // Ready for first clock at position 0
    
    // Don't output pulse/LED here - let first clock handle it
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_DIN;
    isPlaying = true;
    ppqnCounter = 0;  // Ready for first clock at position 0
    
    // Don't output pulse/LED here - let first clock handle it
  }
}

void SyncOut::handleStop(ClockSource source) {
  if (source == CLOCK_SOURCE_USB) {
    usbIsPlaying = false;
    activeSource = CLOCK_SOURCE_NONE;
    avgUSBClockInterval = 0;
    prevUSBClockTime = 0;
    
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_NONE;
    isPlaying = false;
    ppqnCounter = 0;
    
    digitalWrite(CLOCK_OUT_PIN, LOW);
    digitalWrite(LED_BEAT_PIN, LOW);
    clockState = false;
    ledState = false;
  }
}

void SyncOut::update() {
  unsigned long currentTime = micros();
  
  checkUSBTimeout();
  
  if (clockState && (currentTime - lastPulseTime >= PULSE_WIDTH_US)) {
    digitalWrite(CLOCK_OUT_PIN, LOW);
    clockState = false;
  }
  
  if (ledState && (currentTime - lastPulseTime >= PULSE_WIDTH_US)) {
    digitalWrite(LED_BEAT_PIN, LOW);
    ledState = false;
  }
}

void SyncOut::checkUSBTimeout() {
  if (!usbIsPlaying || avgUSBClockInterval == 0) return;
  
  unsigned long now = millis();
  unsigned long timeSinceLastClock = now - lastUSBClockTime;
  unsigned long timeoutThreshold = avgUSBClockInterval * 3;
  
  if (timeSinceLastClock > timeoutThreshold) {
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

bool SyncOut::isJackConnected() {
  return digitalRead(SYNC_DETECT_PIN) == HIGH;
}
