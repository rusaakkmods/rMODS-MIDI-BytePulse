/**
 * MIDI BytePulse - Sync Handler Implementation
 */

#include "Sync.h"
#include "config.h"
#include <MIDIUSB.h>

#define PULSE_WIDTH_US 5000
#define PPQN 24
#define SYNC_OUT_PPQN 24  // Output 24 PPQN (MIDI clock standard - universal compatibility)

void Sync::begin() {
  pinMode(SYNC_OUT_PIN, OUTPUT);
  pinMode(SYNC_OUT_DETECT_PIN, INPUT_PULLUP);
  pinMode(SYNC_IN_PIN, INPUT_PULLUP);
  pinMode(SYNC_IN_DETECT_PIN, INPUT_PULLUP);
  pinMode(LED_BEAT_PIN, OUTPUT);
  digitalWrite(SYNC_OUT_PIN, LOW);
  digitalWrite(LED_BEAT_PIN, LOW);
  
  ppqnCounter = 0;
  isPlaying = false;
  usbIsPlaying = false;
  syncInIsPlaying = false;
  clockState = false;
  ledState = false;
  activeSource = CLOCK_SOURCE_NONE;
  lastUSBClockTime = 0;
  prevUSBClockTime = 0;
  avgUSBClockInterval = 0;
  lastDINClockTime = 0;
  prevDINClockTime = 0;
  avgDINClockInterval = 0;
  lastSyncInTime = 0;
  prevSyncInTime = 0;
  avgSyncInInterval = 0;
  syncInPulseTime = 0;
}

// Called from interrupt - minimal processing only
void Sync::handleSyncInPulse() {
  if (!isSyncInConnected()) return;
  syncInPulseTime = millis();
}

void Sync::handleClock(ClockSource source) {
  unsigned long now = millis();
  
  // Priority: SYNC_IN > USB > DIN
  if (source == CLOCK_SOURCE_DIN && (activeSource == CLOCK_SOURCE_USB || activeSource == CLOCK_SOURCE_SYNC_IN)) {
    return;
  }
  if (source == CLOCK_SOURCE_USB && activeSource == CLOCK_SOURCE_SYNC_IN) {
    return;
  }
  
  // Update timing for active source
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
  if (ppqnCounter % (PPQN / SYNC_OUT_PPQN) == 0) {
    if (isSyncOutConnected()) {
      digitalWrite(SYNC_OUT_PIN, HIGH);
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

void Sync::handleStart(ClockSource source) {
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

void Sync::handleStop(ClockSource source) {
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
    
    digitalWrite(SYNC_OUT_PIN, LOW);
    digitalWrite(LED_BEAT_PIN, LOW);
    clockState = false;
    ledState = false;
  }
}

void Sync::update() {
  unsigned long currentTime = micros();
  
  // Process sync input pulses (from interrupt)
  if (syncInPulseTime > 0) {
    unsigned long pulseTime = syncInPulseTime;
    syncInPulseTime = 0;
    
    if (!syncInIsPlaying) {
      syncInIsPlaying = true;
      isPlaying = true;
      activeSource = CLOCK_SOURCE_SYNC_IN;
      ppqnCounter = 0;
      lastSyncInTime = pulseTime;
      prevSyncInTime = 0;
      avgSyncInInterval = 0;
    }
    
    // Send MIDI Clock to USB (every pulse)
    sendMIDIClock();
    
    // Forward sync input directly to sync output (1:1 passthrough)
    if (isSyncOutConnected()) {
      digitalWrite(SYNC_OUT_PIN, HIGH);
      clockState = true;
      lastPulseTime = currentTime;
    }
    
    // Update timing and LED
    if (syncInIsPlaying) {
      if (prevSyncInTime > 0) {
        unsigned long interval = pulseTime - prevSyncInTime;
        avgSyncInInterval = (avgSyncInInterval == 0) ? interval : (avgSyncInInterval * 3 + interval) / 4;
      }
      prevSyncInTime = pulseTime;
      lastSyncInTime = pulseTime;
      
      // LED flash every 24 pulses (quarter notes)
      if (ppqnCounter == 0) {
        digitalWrite(LED_BEAT_PIN, HIGH);
        ledState = true;
        if (!clockState) lastPulseTime = currentTime;
      }
      
      ppqnCounter++;
      if (ppqnCounter >= PPQN) ppqnCounter = 0;
    }
  }
  
  // Check sync input timeout/disconnect
  if (syncInIsPlaying) {
    if (!isSyncInConnected()) {
      syncInIsPlaying = false;
      if (activeSource == CLOCK_SOURCE_SYNC_IN) {
        activeSource = CLOCK_SOURCE_NONE;
        isPlaying = false;
        ppqnCounter = 0;
        avgSyncInInterval = 0;
        prevSyncInTime = 0;
      }
    }
    else if (avgSyncInInterval > 0 && (millis() - lastSyncInTime) > (avgSyncInInterval * 3)) {
      syncInIsPlaying = false;
      if (activeSource == CLOCK_SOURCE_SYNC_IN) {
        activeSource = CLOCK_SOURCE_NONE;
        isPlaying = false;
        ppqnCounter = 0;
      }
    }
  }
  
  checkUSBTimeout();
  
  // Turn off pulses after 5ms
  if (clockState && (currentTime - lastPulseTime >= PULSE_WIDTH_US)) {
    digitalWrite(SYNC_OUT_PIN, LOW);
    clockState = false;
  }
  
  if (ledState && (currentTime - lastPulseTime >= PULSE_WIDTH_US)) {
    digitalWrite(LED_BEAT_PIN, LOW);
    ledState = false;
  }
}

void Sync::checkUSBTimeout() {
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

void Sync::pulseClock() {
  digitalWrite(SYNC_OUT_PIN, HIGH);
  clockState = true;
  lastPulseTime = micros();
}

void Sync::pulseLED() {
  digitalWrite(LED_BEAT_PIN, HIGH);
  ledState = true;
}

bool Sync::isSyncOutConnected() {
  return digitalRead(SYNC_OUT_DETECT_PIN) == HIGH;
}

bool Sync::isSyncInConnected() {
  return digitalRead(SYNC_IN_DETECT_PIN) == HIGH;
}

void Sync::sendMIDIClock() {
  midiEventPacket_t clockEvent = {0x0F, 0xF8, 0, 0};
  MidiUSB.sendMIDI(clockEvent);
  // No flush here - let main loop handle batching
}
