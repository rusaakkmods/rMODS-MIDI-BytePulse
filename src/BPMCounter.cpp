/**
 * MIDI BytePulse - BPM Counter Implementation
 */

#include "BPMCounter.h"
#include "DisplayControl.h"
#include "config.h"

BPMCounter::BPMCounter() {
}

void BPMCounter::handleBeat() {
  // Update display beat indicator
  if (displayControl) {
    displayControl->beatOn();
    beatOnTime = millis();
    beatIsOn = true;
  }
  
  // When back to first position, calculate BPM
  if (beatPosition == 0) {
    unsigned long now = millis();
    if (lastBeatTime > 0) {
      unsigned long interval = now - lastBeatTime;
      // Interval is for 4 beats (one measure), so divide by 4 to get per-beat interval
      // BPM = 60,000 ms/min / (interval_ms / 4)
      // Simplified: BPM = 240,000 / interval_ms
      currentBPM = 240000UL / interval;
      
      #if SERIAL_DEBUG
      DEBUG_PRINT(interval);
      DEBUG_PRINT("ms = ");
      DEBUG_PRINT(currentBPM);
      DEBUG_PRINTLN(" BPM");
      #endif
    }
    lastBeatTime = now;
  }
  
  // Move to next position for next beat
  beatPosition = (beatPosition + 1) % 4;
}

void BPMCounter::reset() {
  beatPosition = 0;
  lastBeatTime = 0;
  beatIsOn = false;
  // Don't reset currentBPM - keep last known value
  
  // Clear display on reset
  if (displayControl) {
    displayControl->clear();
  }
}

void BPMCounter::update() {
  // Turn off beat indicator after 50ms
  if (beatIsOn && displayControl) {
    if (millis() - beatOnTime >= 50) {
      displayControl->beatOff();
      beatIsOn = false;
    }
  }
}

uint16_t BPMCounter::getBPM() {
  return currentBPM;
}

bool BPMCounter::hasChanged(uint8_t threshold) {
  if (abs((int)currentBPM - (int)lastReportedBPM) >= threshold) {
    lastReportedBPM = currentBPM;
    return true;
  }
  return false;
}
