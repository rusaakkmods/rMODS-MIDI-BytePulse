/**
 * MIDI BytePulse - Display Handler Implementation
 * Non-blocking updates using AceSegment library
 */

#include "Display.h"
#include "config.h"

using ace_tmi::SimpleTmi1637Interface;
using ace_segment::Tm1637Module;

void Display::begin() {
  // Create TM1637 interface with 100us bit delay - guaranteed to work with all modules
  // Even with capacitors, this is the safest setting from AceSegment examples
  tmiInterface = new SimpleTmi1637Interface(DISPLAY_DIO_PIN, DISPLAY_CLK_PIN, 100);
  ledModule = new Tm1637Module<SimpleTmi1637Interface, 4>(*tmiInterface);
  
  tmiInterface->begin();
  ledModule->begin();
  ledModule->setBrightness(2);  // Medium brightness (0-7)
  
  // Total animation: 2000ms - matching original behavior
  uint8_t segments[7] = {
    0b00000001,  // Top
    0b00000010,  // Top right
    0b00000100,  // Bottom right
    0b00001000,  // Bottom
    0b00010000,  // Bottom left
    0b00100000,  // Top left
    0b01000000   // Middle
  };
  
  // Segment cascade (10 frames, 1000ms)
  for (int frame = 0; frame < 10; frame++) {
    for (int digit = 0; digit < 4; digit++) {
      int segmentIdx = frame - digit;
      if (segmentIdx >= 0 && segmentIdx < 7) {
        ledModule->setPatternAt(digit, segments[segmentIdx]);
      } else {
        ledModule->setPatternAt(digit, 0b00000000);
      }
    }
    ledModule->flush();
    delay(100);
  }
  
  // Decimal points (4 frames, 600ms)
  for (int dp = 0; dp < 4; dp++) {
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, digit == dp ? 0b10000000 : 0b00000000);
    }
    ledModule->flush();
    delay(150);
  }
  
  // Blink all (2 blinks, 400ms)
  for (int blink = 0; blink < 2; blink++) {
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, 0xFF);
    }
    ledModule->flush();
    delay(100);
    
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, 0x00);
    }
    ledModule->flush();
    delay(100);
  }
}

void Display::showStandby() {
  if (ledModule) {
    // Show "StbY"
    uint8_t stby[] = {0b01101101, 0b01111000, 0b01111100, 0b01101110};
    for (int i = 0; i < 4; i++) {
      ledModule->setPatternAt(i, stby[i]);
    }
    ledModule->flush();
  }
}

void Display::updateClockIndicator(bool clockRunning) {
  // Not used with AceSegment - decimal is part of BPM pattern
}

void Display::setBPM(uint16_t bpm) {
  bpm = constrain(bpm, 20, 400);
  
  // Only update if changed by more than 2 BPM
  if (abs((int)bpm - (int)currentBPM) > 2) {
    currentBPM = bpm;
  }
  
  // Mark as playing - triggers different animation
  isPlaying = true;
  isIdle = false;
  
  // Don't display BPM - keep display blank during playback
  // BPM calculation still runs in background for potential future use
}

void Display::showPlay() {
  // Don't show "PLaY" - just mark as playing for animation
  isPlaying = true;
  isIdle = false;
}

void Display::showStop() {
  // Show "StoP" briefly when stop detected
  if (ledModule) {
    showingMIDIMessage = true;
    midiMessageTime = millis();
    isIdle = false;  // Temporarily override state to show message
    ledModule->setPatternAt(0, 0b01101101);  // S
    ledModule->setPatternAt(1, 0b01111000);  // t
    ledModule->setPatternAt(2, 0b01011100);  // o
    ledModule->setPatternAt(3, 0b01110011);  // P
  }
}

void Display::advanceAnimation() {
  // Called on each MIDI clock pulse to advance animation
  // MIDI clock = 24 PPQN, 96 pulses per bar (4/4)
  // For 16 steps per bar: advance every 6 pulses (96/16=6)
  if (isPlaying && !isIdle && !showingMIDIMessage) {
    static uint8_t clockCounter = 0;
    static uint8_t stepCounter = 0;
    
    clockCounter++;
    
    if (clockCounter >= 6) {  // Advance every 6 MIDI clock pulses (16th note)
      clockCounter = 0;
      animationNeedsUpdate = true;
      
      // Track beats: every 4 animation steps = 1 beat (24 clock pulses)
      stepCounter++;
      if (stepCounter >= 4) {  // 4 steps = 1 quarter note
        stepCounter = 0;
        currentBeat = (currentBeat + 1) % 4;  // Cycle through beats 0-3
      }
    }
  }
}

void Display::setBeat(uint8_t beat) {
  currentBeat = beat % 4;
}

void Display::setSource(const char* source) {
  // Optional: Show source indicator
}

uint8_t Display::charToSegment(char c) {
  // Convert character to 7-segment pattern
  switch (c) {
    case '0': return 0b00111111;
    case '1': return 0b00000110;
    case '2': return 0b01011011;
    case '3': return 0b01001111;
    case '4': return 0b01100110;
    case '5': return 0b01101101;
    case '6': return 0b01111101;
    case '7': return 0b00000111;
    case '8': return 0b01111111;
    case '9': return 0b01101111;
    case 'A': case 'a': return 0b01110111;
    case 'B': case 'b': return 0b01111100;
    case 'C': case 'c': return 0b00111001;
    case 'D': case 'd': return 0b01011110;
    case 'E': case 'e': return 0b01111001;
    case 'F': case 'f': return 0b01110001;
    case 'L': case 'l': return 0b00111000;
    case 'N': case 'n': return 0b01010100;
    case 'o': case 'O': return 0b01011100;
    case 'P': case 'p': return 0b01110011;
    case 't': case 'T': return 0b01111000;
    case 'Y': case 'y': return 0b01101110;
    case '.': return 0b10000000;  // Decimal point
    case '#': return 0b00000000;  // Placeholder (channel will be shown)
    default: return 0b00000000;
  }
}

void Display::showMIDIMessage(const char* type, uint8_t data, uint8_t channel) {
  // Show MIDI message briefly when idle
  // Note On: "1n.3C" (channel 1, 'n' with decimal, note 0x3C)
  if (ledModule && isIdle) {
    showingMIDIMessage = true;
    midiMessageTime = millis();
    
    // Convert hex data to two chars (e.g., 0x7F -> "7F")
    char hex1 = (data >> 4) < 10 ? '0' + (data >> 4) : 'A' + (data >> 4) - 10;
    char hex2 = (data & 0x0F) < 10 ? '0' + (data & 0x0F) : 'A' + (data & 0x0F) - 10;
    
    // First digit: channel in hex (0-F)
    char channelHex = channel < 10 ? '0' + channel : 'A' + channel - 10;
    
    // Second digit: 'n' with decimal point
    uint8_t nWithDecimal = charToSegment('n') | 0b10000000;
    
    ledModule->setPatternAt(0, charToSegment(channelHex));
    ledModule->setPatternAt(1, nWithDecimal);
    ledModule->setPatternAt(2, charToSegment(hex1));
    ledModule->setPatternAt(3, charToSegment(hex2));
  }
}

void Display::clear() {
  if (ledModule) {
    // Start idle animation
    isIdle = true;
    isPlaying = false;
    idleAnimFrame = 0;
    lastIdleAnimTime = millis();
    currentBPM = 0;
  }
}

void Display::flush() {
  // Call flushIncremental() every 20ms like the AceSegment examples
  // This updates one digit at a time in a round-robin fashion
  if (ledModule) {
    unsigned long now = millis();
    if ((unsigned long)(now - lastFlushTime) >= 20) {
      lastFlushTime = now;
      ledModule->flushIncremental();
    }
    
    // Check if we should resume normal display after showing transport message
    if (showingMIDIMessage && (unsigned long)(now - midiMessageTime) >= 500) {
      showingMIDIMessage = false;
      // Clear display when transport message times out
      for (int i = 0; i < 4; i++) {
        ledModule->setPatternAt(i, 0b00000000);
      }
    }
    
    // Handle idle animation
    if (isIdle && (unsigned long)(now - lastIdleAnimTime) >= 100) {
      lastIdleAnimTime = now;
      
      // Only animate if not showing MIDI message
      if (!showingMIDIMessage) {
        // Complex chaotic pattern (16 frames) - IDLE pattern
        const uint8_t chaoticPattern[16] = {
          0b00000001,  // Top
          0b00100001,  // Top + top-left
          0b00100000,  // Top-left
          0b00110000,  // Top-left + bottom-left
          0b00010000,  // Bottom-left
          0b00011000,  // Bottom-left + bottom
          0b00001000,  // Bottom
          0b00001100,  // Bottom + bottom-right
          0b00000100,  // Bottom-right
          0b00000110,  // Bottom-right + top-right
          0b00000010,  // Top-right
          0b01000010,  // Top-right + middle
          0b01000000,  // Middle
          0b01100000,  // Middle + top-left
          0b01010000,  // Middle + bottom-left
          0b01001000   // Middle + bottom
        };
        
        // Show chaotic pattern on all 4 digits with different offsets
        for (int i = 0; i < 4; i++) {
          uint8_t frame = (idleAnimFrame + (i * 4)) % 16;  // Each digit 4 steps offset
          ledModule->setPatternAt(i, chaoticPattern[frame]);
        }
        
        idleAnimFrame = (idleAnimFrame + 1) % 16;
      }
    }
    
    // Handle playing animation (synced to MIDI clock)
    if (isPlaying && !isIdle && !showingMIDIMessage && animationNeedsUpdate) {
      animationNeedsUpdate = false;
      
      // Single rotating pattern shared by all digits - PLAYING pattern
      // Each frame has 1 LED rotating around the segments
      const uint8_t rotatePattern[4] = {
        0b00000001,  // Top
        0b00000010,  // Top-right
        0b00001000,  // Bottom
        0b00100000   // Top-left
      };
      
      // Calculate which digit gets the decimal point based on beat (0-3)
      // Decimal shows on digit corresponding to current beat
      uint8_t decimalDigit = currentBeat;  // Beat 0=digit 0, Beat 1=digit 1, etc.
      
      // All digits use same pattern starting at the same point
      for (int i = 0; i < 4; i++) {
        uint8_t frame = idleAnimFrame % 4;  // All digits show same frame
        uint8_t pattern = rotatePattern[frame];
        
        // Add decimal point to the digit corresponding to current beat
        if (i == decimalDigit) {
          pattern |= 0b10000000;  // Set decimal point bit
        }
        
        ledModule->setPatternAt(i, pattern);
      }
      
      idleAnimFrame = (idleAnimFrame + 1);  // Keep incrementing for decimal tracking
      if (idleAnimFrame >= 16) idleAnimFrame = 0;  // Loop after 16 steps (4 digits × 4 pulses)
    }
  }
}
