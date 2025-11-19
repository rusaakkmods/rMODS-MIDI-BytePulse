/**
 * MIDI BytePulse - BPM Counter Implementation
 */

#include "BPMCounter.h"
#include "HC595Display.h"
#include "PotControl.h"
#include "config.h"

BPMCounter::BPMCounter() {
}

void BPMCounter::handleBeat() {
  // Flag beat for display update in main loop (non-blocking)
  beatOnTime = millis();
  beatNeedsOn = true;
  
  unsigned long now = millis();
  
  // On first beat (position 0), just initialize the timing and continue
  if (beatPosition == 0) {
    if (lastBeatTime == 0) {
      lastBeatTime = now;  // Initialize on first beat
    }
    beatPosition = 1;
    return;  // Don't calculate yet, just set up timing
  }
  
  // On beat 3, capture the interval for deferred calculation (no division here)
  if (beatPosition == 3) {
    if (lastBeatTime > 0) {
      capturedInterval = now - lastBeatTime;
      bpmNeedsCalculation = true;  // Flag for main loop to calculate
      lastBeatTime = now;  // Update for next cycle
    }
    beatPosition = 0;  // Wrap to position 0
  } else {
    beatPosition++;  // Move to next position (1 -> 2 -> 3)
  }
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
  // Calculate BPM if needed (deferred from handleBeat to avoid blocking)
  if (bpmNeedsCalculation) {
    // More accurate calculation with rounding
    // BPM = 60,000 ms/min / (interval_ms / 4 beats)
    // = 240,000 / interval_ms
    // Add 0.5 for rounding: (240,000 + interval/2) / interval
    uint16_t newBPM = (240000UL + (capturedInterval / 2)) / capturedInterval;
    
    // Constrain to valid range: 20-400 BPM
    newBPM = constrain(newBPM, 20, 400);
    
    // Update display if change > 2 BPM or first reading
    if (abs((int)newBPM - (int)currentBPM) > 2 || currentBPM == 0) {
      currentBPM = newBPM;
      bpmNeedsUpdate = true;
    }
    
    #if SERIAL_DEBUG
    DEBUG_PRINT("Interval: ");
    DEBUG_PRINT(capturedInterval);
    DEBUG_PRINT("ms, BPM: ");
    DEBUG_PRINTLN(newBPM);
    #endif
    
    bpmNeedsCalculation = false;
  }
  
  // Update BPM display if changed by threshold
  // BUT don't override volume display if it's active
  if (bpmNeedsUpdate && display) {
    // Check if volume display is active - if so, skip BPM update
    if (!potControl || !potControl->isVolumeDisplayActive()) {
      display->showBPM(currentBPM);
      displayedBPM = currentBPM;
      bpmNeedsUpdate = false;
    }
    // If volume is showing, bpmNeedsUpdate stays true and will update after volume timeout
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
