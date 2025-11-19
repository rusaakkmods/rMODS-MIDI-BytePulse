/**
 * MIDI BytePulse - Transport Control Implementation
 */

#include "TransportControl.h"
#include "HC595Display.h"
#include "BPMCounter.h"
#include "config.h"
#include <MIDIUSB.h>

#define DEBOUNCE_MS 50
#define MESSAGE_DISPLAY_DURATION 500  // Show message for 0.5 second

void TransportControl::begin() {
  pinMode(BTN_PLAY_PIN, INPUT_PULLUP);
  pinMode(BTN_STOP_PIN, INPUT_PULLUP);
  state = TRANSPORT_STOP;
}

void TransportControl::update() {
  handlePlayButton();
  handleStopButton();
  
  // Check if message display timeout expired
  if (messageDisplayActive && display && bpmCounter) {
    unsigned long now = millis();
    if (now - messageDisplayTime >= MESSAGE_DISPLAY_DURATION) {
      messageDisplayActive = false;
      // Restore appropriate display based on state
      if (state == TRANSPORT_STOP) {
        display->showStopped();
        display->setDecimalPoint(3, true);
      } else {
        display->showBPM(bpmCounter->getBPM());
      }
    }
  }
}

void TransportControl::handlePlayButton() {
  bool reading = digitalRead(BTN_PLAY_PIN);
  
  if (reading != lastPlayState) {
    lastPlayDebounce = millis();
  }
  
  if ((millis() - lastPlayDebounce) > DEBOUNCE_MS) {
    if (reading == LOW && !playPressed) {
      playPressed = true;
      switch (state) {
        case TRANSPORT_STOP:
          state = TRANSPORT_PLAY;
          sendStart();
          if (display) {
            display->showPlay();
            messageDisplayActive = true;
            messageDisplayTime = millis();
          }
          break;
        case TRANSPORT_PLAY:
          state = TRANSPORT_PAUSE;
          sendStop();
          if (display) {
            display->showHold();
            messageDisplayActive = true;
            messageDisplayTime = millis();
          }
          break;
        case TRANSPORT_PAUSE:
          state = TRANSPORT_PLAY;
          sendContinue();
          if (display) {
            display->showPlay();
            messageDisplayActive = true;
            messageDisplayTime = millis();
          }
          break;
      }
    } else if (reading == HIGH) {
      playPressed = false;
    }
  }
  
  lastPlayState = reading;
}

void TransportControl::handleStopButton() {
  bool reading = digitalRead(BTN_STOP_PIN);
  
  if (reading != lastStopState) {
    lastStopDebounce = millis();
  }
  
  if ((millis() - lastStopDebounce) > DEBOUNCE_MS) {
    if (reading == LOW && !stopPressed) {
      stopPressed = true;
      state = TRANSPORT_STOP;
      sendStop();
      if (display) {
        display->showStop();
        messageDisplayActive = true;
        messageDisplayTime = millis();
      }
    } else if (reading == HIGH) {
      stopPressed = false;
    }
  }
  
  lastStopState = reading;
}

void TransportControl::sendStart() {
  midiEventPacket_t event = {0x0F, 0xFA, 0, 0};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void TransportControl::sendContinue() {
  midiEventPacket_t event = {0x0F, 0xFB, 0, 0};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void TransportControl::sendStop() {
  midiEventPacket_t event = {0x0F, 0xFC, 0, 0};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
