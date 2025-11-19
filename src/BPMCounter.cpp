/**
 * MIDI BytePulse - BPM Counter Implementation
 */

#include "BPMCounter.h"
// #include "HC595Display.h"  // DISABLED
#include "config.h"

BPMCounter::BPMCounter() {
}

void BPMCounter::handleBeat() {
  // Flag beat for display update in main loop (non-blocking)
  beatOnTime = millis();
  beatNeedsOn = true;
  
  // Calculate BPM when completing a full measure (after beat 3, before wrapping to 0)
  if (beatPosition == 3) {
    unsigned long now = millis();
    if (lastBeatTime > 0) {
      unsigned long interval = now - lastBeatTime;
      // Interval is for 4 beats (one measure)
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
  beatNeedsOn = false;
  beatNeedsOff = false;
  // Don't reset currentBPM - keep last known value
  
  // Display disabled
  // if (display) {
  //   display->clear();
  // }
}

void BPMCounter::update() {
  // Display disabled - no visual feedback
  // if (beatNeedsOn && display) {
  //   display->allDecimalsOff();
  //   display->setDecimalPoint(beatPosition, true);
  //   beatIsOn = true;
  //   beatNeedsOn = false;
  // }
  // 
  // if (beatIsOn && display) {
  //   if (millis() - beatOnTime >= 50) {
  //     display->allDecimalsOff();
  //     beatIsOn = false;
  //   }
  // }
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
