/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Arduino.h>
#include <SoftwareSerial/SoftwareSerial.h>
#include <EasyVR/EasyVR.h>
#include <device.h>

const int NUM_BUTTONS = 10;

const int INFO_LED_PIN = 10;
const int SYS_BUTTON_PIN = 11;
const int PIN_WIFLY_RXD = 0;
const int PIN_WIFLY_TXD = 1;
const int PIN_USB_RXD = 2;
const int PIN_USB_TXD = 3;
const int PIN_EASYVR_RXD = 5;
const int PIN_EASYVR_TXD = 4;

const long USB_BAUD_RATE = 9600;
const long EASYVR_BAUD_RATE = 9600;

//SoftwareSerial usbSerial(PIN_USB_RXD, PIN_USB_TXD);
SoftwareSerial easyvrSerial(PIN_EASYVR_RXD, PIN_EASYVR_TXD);
EasyVR easyvr(easyvrSerial);

int wordGroup = -1;
int wordIndex = -1;

/** States of the device state engine */
enum State {
  RECOGNIZE_TRIGGER, RECOGNIZE_GROUP, RECOGNIZE_WORD
};

/** Current device mode state */
State state = RECOGNIZE_TRIGGER;

DeviceDescriptor device;

void blinkLed();
void error(int num);
void register_message_handlers();
void sendButtonState(uint8_t button);
void buttonRead();
void send_server_register_params();

/**
 * Main system setup.
 */
void setup() {
  pinMode(INFO_LED_PIN, OUTPUT);
  digitalWrite(INFO_LED_PIN, HIGH);
  delay(500);

  easyvrSerial.begin(EASYVR_BAUD_RATE);
  if (!easyvr.detect()) {
    error(1);
  }
  easyvr.setTimeout(0);
  easyvr.setLanguage(EasyVR::ENGLISH);

//	usbSerial.begin(USB_BAUD_RATE);

  digitalWrite(INFO_LED_PIN, LOW);
  easyvr.playSound(2, EasyVR::VOL_DOUBLE);
  easyvr.recognizeCommand(0);

  device.type = "EasyVr";
  device.description = "EadyVr Speech Interface";
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.registerMessageHandlers = register_message_handlers;
  device.sendServerRegisterParams = send_server_register_params;
  deviceInit(device);
}

/**
 * Main system loop.
 */
void loop() {
  switch (state) {
    case RECOGNIZE_TRIGGER:
      if (easyvr.hasFinished()) {
        blinkLed();
        int wordTrigger = easyvr.getCommand();
        if (wordTrigger >= 0) {
          easyvr.recognizeCommand(1);
          state = RECOGNIZE_GROUP;
        } else {
          easyvr.playSound(2, EasyVR::VOL_DOUBLE);
          easyvr.recognizeCommand(0);
        }
      }
      break;

    case RECOGNIZE_GROUP:
      if (easyvr.hasFinished()) {
        blinkLed();
        wordGroup = easyvr.getCommand();
        if (wordGroup >= 0) {
          easyvr.recognizeCommand(1 + wordGroup);
          state = RECOGNIZE_WORD;
        } else {
          easyvr.playSound(2, EasyVR::VOL_DOUBLE);
          easyvr.recognizeCommand(0);
          state = RECOGNIZE_TRIGGER;
        }
      }
      break;

    case RECOGNIZE_WORD:
      if (easyvr.hasFinished()) {
        blinkLed();
        wordIndex = easyvr.getCommand();
        if (wordIndex >= 0) {
          sendButtonState(wordIndex - 1);
          easyvr.recognizeCommand(0);
          state = RECOGNIZE_TRIGGER;
        } else {
          easyvr.playSound(2, EasyVR::VOL_DOUBLE);
          easyvr.recognizeCommand(0);
          state = RECOGNIZE_TRIGGER;
        }
      }
      break;
  }
}

/**
 * Register device specific message handlers.
 */
void register_message_handlers() {
  device.messenger->attach(MSG_BUTTON_READ, buttonRead);
}

/**
 * Blinke the LED <num> times in case of an error.
 *
 * @param num Error number
 */
void error(int num) {
  for (;;) {
    for (int i = 0; i < num; ++i) {
      digitalWrite(INFO_LED_PIN, HIGH);
      delay(250);
      digitalWrite(INFO_LED_PIN, LOW);
      delay(250);
    }
    delay(1000);
  }
}

/**
 * Blink the LED.
 */
void blinkLed() {
  digitalWrite(INFO_LED_PIN, HIGH);
  delay(50);
  digitalWrite(INFO_LED_PIN, LOW);
}

void sendButtonState(uint8_t button) {
  device.messenger->sendCmdStart(MSG_BUTTON_STATE);
  device.messenger->sendCmdArg(button);
  device.messenger->sendCmdArg(BUTTON_PRESSED);
  device.messenger->sendCmdEnd();
  deviceWiflyFlush();
  delay(500);
  device.messenger->sendCmdStart(MSG_BUTTON_STATE);
  device.messenger->sendCmdArg(button);
  device.messenger->sendCmdArg(BUTTON_RELEASED);
  device.messenger->sendCmdEnd();
  deviceWiflyFlush();
}

/**
 * Called when a MSG_BUTTON_READ was received.
 */
void buttonRead() {
}

/**
 * Send additional parameter with the MSG_REGISTER_REQUEST.
 *
 * @param messenger
 */
void send_server_register_params() {
  device.messenger->sendCmdArg(NUM_BUTTONS);
}
