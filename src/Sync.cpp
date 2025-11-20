#include "Sync.h"
#include "Display.h"
#include "config.h"
#include <MIDIUSB.h>
#include <MIDI.h>

extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDI_DIN;

#define PULSE_WIDTH_US 5000
#define PPQN 24

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
  beatPosition = 0;
  lastBeatTime = 0;
  currentBPM = 0;
  lastDisplayedBPM = 0;
}

void Sync::handleSyncInPulse() {
  if (!isSyncInConnected()) return;
  syncInPulseTime = millis();
}

void Sync::handleClock(ClockSource source) {
  unsigned long now = millis();
  
  if (source == CLOCK_SOURCE_DIN && (activeSource == CLOCK_SOURCE_USB || activeSource == CLOCK_SOURCE_SYNC_IN)) {
    return;
  }
  if (source == CLOCK_SOURCE_USB && activeSource == CLOCK_SOURCE_SYNC_IN) {
    return;
  }
  
  if (source == CLOCK_SOURCE_USB && usbIsPlaying) {
    if (prevUSBClockTime > 0) {
      unsigned long interval = now - prevUSBClockTime;
      
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
      ppqnCounter = 0;
      lastDINClockTime = millis();
      prevDINClockTime = 0;
      avgDINClockInterval = 0;
    }
  }
  
  if (!isPlaying) return;
  
  if (display) {
    display->advanceAnimation();
  }
  
  if (isSyncOutConnected()) {
    digitalWrite(SYNC_OUT_PIN, HIGH);
    clockState = true;
    lastPulseTime = micros();
  }
  
  if (ppqnCounter == 0) {
    digitalWrite(LED_BEAT_PIN, HIGH);
    ledState = true;
    if (!clockState) {
      lastPulseTime = micros();
    }
    
    unsigned long now = millis();
    
    if (beatPosition == 0 && lastBeatTime == 0) {
      lastBeatTime = now;
    }
    
    if (beatPosition == 3) {
      if (lastBeatTime > 0) {
        unsigned long interval = now - lastBeatTime;
        currentBPM = 240000UL / interval;
        
        #if SERIAL_DEBUG
        if (abs((int)currentBPM - (int)lastDisplayedBPM) > 2) {
          DEBUG_PRINT("BPM: ");
          DEBUG_PRINTLN(currentBPM);
          lastDisplayedBPM = currentBPM;
          
          if (onBPMUpdate) {
            onBPMUpdate(currentBPM);
          }
        }
        #else
        if (abs((int)currentBPM - (int)lastDisplayedBPM) > 2) {
          lastDisplayedBPM = currentBPM;
          if (onBPMUpdate) {
            onBPMUpdate(currentBPM);
          }
        }
        #endif
        
        lastBeatTime = now;
      }
    }
    
    beatPosition = (beatPosition + 1) % 4;
  }
  
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
    ppqnCounter = 0;
    beatPosition = 0;
    lastBeatTime = 0;
    lastDisplayedBPM = 0;
    
    MIDI_DIN.sendRealTime(midi::Start);
    
    if (onClockStart) {
      onClockStart();
    }
    
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_DIN;
    isPlaying = true;
    ppqnCounter = 0;
    beatPosition = 0;
    lastBeatTime = 0;
    lastDisplayedBPM = 0;
    
    MIDI_DIN.sendRealTime(midi::Start);
    
    if (onClockStart) {
      onClockStart();
    }
  }
}

void Sync::handleStop(ClockSource source) {
  if (source == CLOCK_SOURCE_USB) {
    usbIsPlaying = false;
    isPlaying = false;
    activeSource = CLOCK_SOURCE_NONE;
    avgUSBClockInterval = 0;
    prevUSBClockTime = 0;
    ppqnCounter = 0;
    beatPosition = 0;
    lastBeatTime = 0;
    
    MIDI_DIN.sendRealTime(midi::Stop);
    
    if (onClockStop) {
      onClockStop();
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
    beatPosition = 0;
    lastBeatTime = 0;
    
    digitalWrite(SYNC_OUT_PIN, LOW);
    digitalWrite(LED_BEAT_PIN, LOW);
    clockState = false;
    ledState = false;
    
    MIDI_DIN.sendRealTime(midi::Stop);
    
    if (onClockStop) {
      onClockStop();
    }
  }
}

void Sync::update() {
  unsigned long currentTime = micros();
  
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
      beatPosition = 0;
      lastBeatTime = 0;
      lastDisplayedBPM = 0;
      
      if (onClockStart) {
        onClockStart();
      }
    }
    
    sendMIDIClock();
    
    if (display) {
      display->advanceAnimation();
    }
    
    if (isSyncOutConnected()) {
      digitalWrite(SYNC_OUT_PIN, HIGH);
      clockState = true;
      lastPulseTime = currentTime;
    }
    
    if (syncInIsPlaying) {
      if (prevSyncInTime > 0) {
        unsigned long interval = pulseTime - prevSyncInTime;
        avgSyncInInterval = (avgSyncInInterval == 0) ? interval : (avgSyncInInterval * 3 + interval) / 4;
      }
      prevSyncInTime = pulseTime;
      lastSyncInTime = pulseTime;
      
      if (ppqnCounter == 0) {
        digitalWrite(LED_BEAT_PIN, HIGH);
        ledState = true;
        if (!clockState) lastPulseTime = currentTime;
        
        unsigned long now = millis();
        if (beatPosition == 3) {
          if (lastBeatTime > 0) {
            unsigned long interval = now - lastBeatTime;
            currentBPM = 240000UL / interval;
            
            #if SERIAL_DEBUG
            if (abs((int)currentBPM - (int)lastDisplayedBPM) > 2) {
              DEBUG_PRINT("BPM: ");
              DEBUG_PRINTLN(currentBPM);
              lastDisplayedBPM = currentBPM;
              
              if (onBPMUpdate) {
                onBPMUpdate(currentBPM);
              }
            }
            #else
            if (abs((int)currentBPM - (int)lastDisplayedBPM) > 2) {
              lastDisplayedBPM = currentBPM;
              if (onBPMUpdate) {
                onBPMUpdate(currentBPM);
              }
            }
            #endif
          }
          lastBeatTime = now;
        }
        beatPosition = (beatPosition + 1) % 4;
      }
      
      ppqnCounter++;
      if (ppqnCounter >= PPQN) ppqnCounter = 0;
    }
  }
  
  if (syncInIsPlaying) {
    if (!isSyncInConnected()) {
      syncInIsPlaying = false;
      if (activeSource == CLOCK_SOURCE_SYNC_IN) {
        activeSource = CLOCK_SOURCE_NONE;
        isPlaying = false;
        ppqnCounter = 0;
        avgSyncInInterval = 0;
        prevSyncInTime = 0;
        beatPosition = 0;
        lastBeatTime = 0;
        
        if (onClockStop) {
          onClockStop();
        }
      }
    }
    else if (avgSyncInInterval > 0 && (millis() - lastSyncInTime) > (avgSyncInInterval * 3)) {
      syncInIsPlaying = false;
      if (activeSource == CLOCK_SOURCE_SYNC_IN) {
        activeSource = CLOCK_SOURCE_NONE;
        isPlaying = false;
        ppqnCounter = 0;
        beatPosition = 0;
        lastBeatTime = 0;
        
        if (onClockStop) {
          onClockStop();
        }
      }
    }
  }
  
  checkUSBTimeout();
  
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
  if ((now - lastUSBClockTime) > (avgUSBClockInterval * 3)) {
    usbIsPlaying = false;
    isPlaying = false;
    activeSource = CLOCK_SOURCE_NONE;
    avgUSBClockInterval = 0;
    prevUSBClockTime = 0;
    ppqnCounter = 0;
    beatPosition = 0;
    lastBeatTime = 0;
    
    if (onClockStop) {
      onClockStop();
    }
  }
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
  MIDI_DIN.sendRealTime(midi::Clock);
}
