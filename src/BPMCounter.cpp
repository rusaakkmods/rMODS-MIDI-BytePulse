/**
 * MIDI BytePulse - BPM Counter Implementation
 */

#include "BPMCounter.h"
#include "HC595Display.h"
#include "config.h"

BPMCounter::BPMCounter() {
}

void BPMCounter::handleBeat() {
  // Flag beat for display update in main loop (non-blocking)
  beatOnTime = millis();
  beatNeedsOn = true;
  
  unsigned long now = millis();
  
  // On first beat (position 0), initialize the timing
  if (beatPosition == 0 && lastBeatTime == 0) {
    lastBeatTime = now;
  }
  
  // Calculate BPM every 4 beats (after beat 3, before wrapping to 0)
  if (beatPosition == 3) {
    if (lastBeatTime > 0) {
      unsigned long interval = now - lastBeatTime;
      
      // More accurate calculation with rounding
      // BPM = 60,000 ms/min / (interval_ms / 4 beats)
      // = 240,000 / interval_ms
      // Add 0.5 for rounding: (240,000 + interval/2) / interval
      uint16_t newBPM = (240000UL + (interval / 2)) / interval;
      
      // Constrain to valid range: 20-400 BPM
      newBPM = constrain(newBPM, 20, 400);
      
      // Update display every 4 beats if change > 2 BPM or first reading
      if (abs((int)newBPM - (int)currentBPM) > 2 || currentBPM == 0) {
        currentBPM = newBPM;
        bpmNeedsUpdate = true;
      }
      
      #if SERIAL_DEBUG
      DEBUG_PRINT("Interval: ");
      DEBUG_PRINT(interval);
      DEBUG_PRINT("ms, BPM: ");
      DEBUG_PRINTLN(newBPM);
      #endif
    }
    
    // Update lastBeatTime for next calculation
    lastBeatTime = now;
  }
  
  // Move to next position for next beat
  beatPosition = (beatPosition + 1) % 4;
}

void BPMCounter::reset() {
  beatPosition = 0;
  lastBeatTime = 0;
  beatIsOn = false;
  beatNeedsOn = false;
  beatNeedsOff = false;
  bpmNeedsUpdate = false;
  currentBPM = 0;  // Reset BPM on stop
  displayedBPM = 0;
  
  // Show "-" with decimal when stopped
  if (display) {
    display->showStopped();
    display->setDecimalPoint(3, true);
  }
}

void BPMCounter::start() {
  // Show "t" immediately when playback starts (even before BPM calculation)
  if (display) {
    display->showBPM(0);  // Shows "t  0" or similar until first BPM is calculated
  }
}

void BPMCounter::update() {
  // Update BPM display if changed by threshold
  if (bpmNeedsUpdate && display) {
    display->showBPM(currentBPM);
    displayedBPM = currentBPM;
    bpmNeedsUpdate = false;
  }
  
  // Turn on 4th digit decimal on beat
  if (beatNeedsOn && display) {
    // Turn on decimal point on 4th digit (index 3)
    display->setDecimalPoint(3, true);
    beatIsOn = true;
    beatNeedsOn = false;
  }
  
  // Turn off 4th digit decimal after 50ms
  if (beatIsOn && display) {
    if (millis() - beatOnTime >= 50) {
      display->setDecimalPoint(3, false);
      beatIsOn = false;
    }
  }
}

uint16_t BPMCounter::getBPM() {
  return currentBPM;
}

bool BPMCounter::hasChanged(uint8_t threshold) {
  if (abs((int)currentBPM - (int)displayedBPM) >= threshold) {
    return true;
  }
  return false;
}
