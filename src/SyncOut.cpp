/**
 * MIDI BytePulse - Sync Output Implementation
 */

#include "SyncOut.h"
#include "BPMCounter.h"
#include "config.h"

#define PULSE_WIDTH_US 5000
#define PPQN 24

void SyncOut::begin() {
  pinMode(CLOCK_OUT_PIN, OUTPUT);
  pinMode(SYNC_DETECT_PIN, INPUT_PULLUP);
  pinMode(LED_BEAT_PIN, OUTPUT);
  digitalWrite(CLOCK_OUT_PIN, LOW);
  digitalWrite(LED_BEAT_PIN, LOW);
  
  ppqnCounter = 0;
  displayUpdateCounter = 0;
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
      ppqnCounter = 1;  // Start at 1, not 0, so first clock doesn't trigger LED
      lastDINClockTime = millis();
      prevDINClockTime = 0;
      avgDINClockInterval = 0;
      
      // Show "t" immediately when DIN starts without START message
      if (bpmCounter) {
        bpmCounter->reset();
        bpmCounter->start();
      }
      
      // Send first clock pulse but skip LED/beat
      if (isJackConnected()) {
        digitalWrite(CLOCK_OUT_PIN, HIGH);
      }
      clockState = true;
      lastPulseTime = micros();
      return;  // Skip the LED check for initialization clock
    }
  }
  
  if (!isPlaying) return;
  
  if (isJackConnected()) {
    digitalWrite(CLOCK_OUT_PIN, HIGH);
  }
  clockState = true;
  lastPulseTime = micros();
  
  if (ppqnCounter == 0) {
    digitalWrite(LED_BEAT_PIN, HIGH);
    ledState = true;
    
    // Update BPM counter on downbeat
    if (bpmCounter) {
      bpmCounter->handleBeat();
    }
  }
  
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
    ppqnCounter = 0;
    
    if (bpmCounter) {
      bpmCounter->reset();
      bpmCounter->start();  // Show "t" immediately
    }
    
    if (isJackConnected()) {
      digitalWrite(CLOCK_OUT_PIN, HIGH);
    }
    clockState = true;
    lastPulseTime = micros();
    digitalWrite(LED_BEAT_PIN, HIGH);
    ledState = true;
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_DIN;
    isPlaying = true;
    ppqnCounter = 0;
    
    if (bpmCounter) {
      bpmCounter->reset();
      bpmCounter->start();  // Show "t" immediately
    }
    
    if (isJackConnected()) {
      digitalWrite(CLOCK_OUT_PIN, HIGH);
    }
    clockState = true;
    lastPulseTime = micros();
    digitalWrite(LED_BEAT_PIN, HIGH);
    ledState = true;
  }
}

void SyncOut::handleStop(ClockSource source) {
  if (source == CLOCK_SOURCE_USB) {
    usbIsPlaying = false;
    activeSource = CLOCK_SOURCE_NONE;
    avgUSBClockInterval = 0;
    prevUSBClockTime = 0;
    
    if (bpmCounter) {
      bpmCounter->reset();
    }
    
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
    
    if (bpmCounter) {
      bpmCounter->reset();
    }
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
