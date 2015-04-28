/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Wire.h>
#include <SoftwareSerial.h>
#include <EEPROMex.h>
#include <WiFly.h>
#include <CmdMessenger.h>
#include <device.h>
#include <Bounce.h>

const uint8_t BUTTON_01_PIN = 11;
const uint8_t BUTTON_02_PIN = 9;
const uint8_t BUTTON_03_PIN = 10;
const uint8_t BUTTON_04_PIN = 8;
const uint8_t BUTTON_05_PIN = 0;
const uint8_t BUTTON_06_PIN = 0;
const uint8_t BUTTON_07_PIN = 0;
const uint8_t BUTTON_08_PIN = 0;
const uint8_t BUTTON_09_PIN = 0;
const uint8_t BUTTON_10_PIN = 0;
const uint8_t INFO_LED_PIN = 13;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t NUM_BUTTONS = 4;

uint8_t buttonPins[] = { BUTTON_01_PIN, BUTTON_02_PIN, BUTTON_03_PIN, BUTTON_04_PIN, BUTTON_05_PIN, BUTTON_06_PIN, BUTTON_07_PIN, BUTTON_08_PIN,
    BUTTON_09_PIN, BUTTON_10_PIN };

Bounce buttons[] = { Bounce(BUTTON_01_PIN, 5), Bounce(BUTTON_02_PIN, 5), Bounce(BUTTON_03_PIN, 5), Bounce(BUTTON_04_PIN, 5), Bounce(
    BUTTON_05_PIN, 5), Bounce(BUTTON_06_PIN, 5), Bounce(BUTTON_07_PIN, 5), Bounce(BUTTON_08_PIN, 5), Bounce(BUTTON_09_PIN, 5), Bounce(
    BUTTON_10_PIN, 5) };

const unsigned long INFO_LED_BLINK_MILLIS = 250;

unsigned long infoLedOffMillis;

DeviceDescriptor device;

void setup() {
  device.type = "RemoteControl";
  device.description = "4 buttons demo shield";
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.sendServerRegisterParams = send_server_register_params;
  deviceInit(device);

  pinMode(INFO_LED_PIN, OUTPUT);
  digitalWrite(INFO_LED_PIN, LOW);
  for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
    pinMode(buttonPins[i], INPUT);
    digitalWrite(buttonPins[i], HIGH);
  }
}

void loop() {
  deviceUpdate();

  if (deviceIsOperational()) {
    if (millis() > infoLedOffMillis) {
      digitalWrite(INFO_LED_PIN, LOW);
    }

    for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
      buttons[i].update();

      if (buttons[i].fallingEdge() || buttons[i].risingEdge()) {

        digitalWrite(INFO_LED_PIN, HIGH);
        infoLedOffMillis = millis() + INFO_LED_BLINK_MILLIS;

        int state = buttons[i].read();

        device.messenger->sendCmdStart(MSG_BUTTON_STATE);
        device.messenger->sendCmdArg(i);
        device.messenger->sendCmdArg(state == LOW ? 1 : 0);
        device.messenger->sendCmdEnd();
      }
    }
  }
}

void send_server_register_params() {
  device.messenger->sendCmdArg(NUM_BUTTONS);
}

