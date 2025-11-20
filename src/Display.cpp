#include "Display.h"
#include "config.h"

using ace_tmi::SimpleTmi1637Interface;
using ace_segment::Tm1637Module;

void Display::begin() {
  tmiInterface = new SimpleTmi1637Interface(DISPLAY_DIO_PIN, DISPLAY_CLK_PIN, 100);
  ledModule = new Tm1637Module<SimpleTmi1637Interface, 4>(*tmiInterface);
  
  tmiInterface->begin();
  ledModule->begin();
  ledModule->setBrightness(2);
  
  uint8_t segments[7] = {
    0b00000001, 0b00000010, 0b00000100, 0b00001000,
    0b00010000, 0b00100000, 0b01000000
  };
  
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
  
  for (int dp = 0; dp < 4; dp++) {
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, digit == dp ? 0b10000000 : 0b00000000);
    }
    ledModule->flush();
    delay(150);
  }
  
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

void Display::setBPM(uint16_t bpm) {
  bpm = constrain(bpm, 20, 400);
  
  if (abs((int)bpm - (int)currentBPM) > 2) {
    currentBPM = bpm;
  }
  
  isPlaying = true;
  isIdle = false;
}

void Display::showPlay() {
  isPlaying = true;
  isIdle = false;
}

void Display::showStop() {
  if (ledModule) {
    showingMIDIMessage = true;
    midiMessageTime = millis();
    isIdle = false;
    ledModule->setPatternAt(0, 0b01101101);
    ledModule->setPatternAt(1, 0b01111000);
    ledModule->setPatternAt(2, 0b01011100);
    ledModule->setPatternAt(3, 0b01110011);
  }
  currentBeat = 0;
}

void Display::advanceAnimation() {
  if (isPlaying && !isIdle && !showingMIDIMessage) {
    static uint8_t clockCounter = 0;
    static uint8_t stepCounter = 0;
    
    clockCounter++;
    
    if (clockCounter >= 6) {
      clockCounter = 0;
      animationNeedsUpdate = true;
      
      stepCounter++;
      if (stepCounter >= 4) {
        stepCounter = 0;
        currentBeat = (currentBeat + 1) % 4;
      }
    }
  }
}

void Display::setBeat(uint8_t beat) {
  currentBeat = beat % 4;
}

void Display::showBPM() {
  if (!ledModule) return;
  
  if (!isPlaying) {
    showIdle();
    return;
  }
  
  uint16_t bpm = currentBPM;
  uint8_t digit1 = (bpm / 100) % 10;
  uint8_t digit2 = (bpm / 10) % 10;
  uint8_t digit3 = bpm % 10;
  
  uint8_t tWithDecimal = charToSegment('t') | 0b10000000;
  ledModule->setPatternAt(0, tWithDecimal);
  ledModule->setPatternAt(1, charToSegment('0' + digit1));
  ledModule->setPatternAt(2, charToSegment('0' + digit2));
  ledModule->setPatternAt(3, charToSegment('0' + digit3));
  ledModule->flush();
}

void Display::showIdle() {
  if (!ledModule) return;
  
  ledModule->setPatternAt(0, charToSegment('I'));
  ledModule->setPatternAt(1, charToSegment('d'));
  ledModule->setPatternAt(2, charToSegment('L'));
  ledModule->setPatternAt(3, charToSegment('e'));
  ledModule->flush();
}

void Display::setButtonPressed(bool pressed) {
  buttonPressed = pressed;
}

uint8_t Display::charToSegment(char c) {
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
    case 'I': case 'i': return 0b00000110;
    case 'L': case 'l': return 0b00111000;
    case 'N': case 'n': return 0b01010100;
    case 'o': case 'O': return 0b01011100;
    case 'P': case 'p': return 0b01110011;
    case 't': case 'T': return 0b01111000;
    case 'Y': case 'y': return 0b01101110;
    case '.': return 0b10000000;
    case '#': return 0b00000000;
    default: return 0b00000000;
  }
}

void Display::showMIDIMessage(const char* type, uint8_t data, uint8_t channel) {
  if (ledModule && isIdle) {
    showingMIDIMessage = true;
    midiMessageTime = millis();
    
    char hex1 = (data >> 4) < 10 ? '0' + (data >> 4) : 'A' + (data >> 4) - 10;
    char hex2 = (data & 0x0F) < 10 ? '0' + (data & 0x0F) : 'A' + (data & 0x0F) - 10;
    char channelHex = channel < 10 ? '0' + channel : 'A' + channel - 10;
    uint8_t nWithDecimal = charToSegment('n') | 0b10000000;
    
    ledModule->setPatternAt(0, charToSegment(channelHex));
    ledModule->setPatternAt(1, nWithDecimal);
    ledModule->setPatternAt(2, charToSegment(hex1));
    ledModule->setPatternAt(3, charToSegment(hex2));
  }
}

void Display::clear() {
  if (ledModule) {
    isIdle = true;
    isPlaying = false;
    idleAnimFrame = 0;
    lastIdleAnimTime = millis();
    currentBPM = 0;
  }
}

void Display::flush() {
  if (ledModule) {
    unsigned long now = millis();
    if ((unsigned long)(now - lastFlushTime) >= 20) {
      lastFlushTime = now;
      ledModule->flushIncremental();
    }
    
    if (buttonPressed) {
      return;
    }
    
    if (showingMIDIMessage && (unsigned long)(now - midiMessageTime) >= 500) {
      showingMIDIMessage = false;
      for (int i = 0; i < 4; i++) {
        ledModule->setPatternAt(i, 0b00000000);
      }
    }
    
    if (isIdle && (unsigned long)(now - lastIdleAnimTime) >= 100) {
      lastIdleAnimTime = now;
      
      if (!showingMIDIMessage) {
        const uint8_t chaoticPattern[16] = {
          0b00000001, 0b00100001, 0b00100000, 0b00110000,
          0b00010000, 0b00011000, 0b00001000, 0b00001100,
          0b00000100, 0b00000110, 0b00000010, 0b01000010,
          0b01000000, 0b01100000, 0b01010000, 0b01001000
        };
        
        for (int i = 0; i < 4; i++) {
          uint8_t frame = (idleAnimFrame + (i * 4)) % 16;
          ledModule->setPatternAt(i, chaoticPattern[frame]);
        }
        
        idleAnimFrame = (idleAnimFrame + 1) % 16;
      }
    }
    
    if (isPlaying && !isIdle && !showingMIDIMessage && animationNeedsUpdate) {
      animationNeedsUpdate = false;
      
      const uint8_t rotatePattern[4] = {
        0b00000001, 0b00000010, 0b00001000, 0b00100000
      };
      
      uint8_t decimalDigit = currentBeat;
      
      for (int i = 0; i < 4; i++) {
        uint8_t frame = idleAnimFrame % 4;
        uint8_t pattern = rotatePattern[frame];
        
        if (i == decimalDigit) {
          pattern |= 0b10000000;
        }
        
        ledModule->setPatternAt(i, pattern);
      }
      
      idleAnimFrame = (idleAnimFrame + 1);
      if (idleAnimFrame >= 16) idleAnimFrame = 0;
    }
  }
}
