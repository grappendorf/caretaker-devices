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
#include "ClickEncoder.h"
#include "TimerOne.h"
#include "MultiMap.h"

const uint8_t ENCODER_A_PIN = 11;
const uint8_t ENCODER_B_PIN = 10;
const uint8_t ENCODER_BUTTON_PIN = 8;
const uint8_t VALUE_LED_PIN = 9;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t INFO_LED_PIN = 13;

const int MINIMUM_VALUE = 0;
const int MAXIMUM_VALUE = 255;
const int DELTA_FACTOR = 4;

DeviceDescriptor device;

ClickEncoder encoder(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BUTTON_PIN);
int value, lastValue;

int ledValueIn[] = { 0, 50, 100, 150, 200, 255 };
int ledValueOut[] = { 0, 20, 40, 100, 200, 254 };

void setup() {
  device.type = "RotaryKnob";
  device.description = "Rotary knob demo shield";
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.registerMessageHandlers = register_message_handlers;
  deviceInit(device);

  pinMode(VALUE_LED_PIN, OUTPUT);
  analogWrite(VALUE_LED_PIN, 0);

  value = MINIMUM_VALUE;
  lastValue = MINIMUM_VALUE;
  encoder.setAccelerationEnabled(true);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
}

void loop() {
  deviceUpdate();

  if (deviceIsOperational()) {
    int newValue = value;

    if (encoder.getButton() == ClickEncoder::Clicked) {
      if (value > MINIMUM_VALUE) {
        newValue = MINIMUM_VALUE;
      } else {
        newValue = MAXIMUM_VALUE;
      }
    }

    newValue += encoder.getValue() * DELTA_FACTOR;

    setValue(newValue);
  }
}

void timerIsr() {
  encoder.service();
}

void setValue(int newValue) {
  value = max(min(newValue, MAXIMUM_VALUE), MINIMUM_VALUE);
  if (value != lastValue) {
    DEBUG_PRINT(value);
    DEBUG_PRINT("  ");
    DEBUG_PRINTLN(multiMap<int>(value, ledValueIn, ledValueOut, 6));
    analogWrite(VALUE_LED_PIN, multiMap<int>(value, ledValueIn, ledValueOut, 6));
    rotary_read();
    lastValue = value;
  }
}

void register_message_handlers() {
  device.messenger->attach(MSG_ROTARY_WRITE, rotary_write);
  device.messenger->attach(MSG_ROTARY_READ, rotary_read);
}

void rotary_read() {
  device.messenger->sendCmdStart(MSG_ROTARY_STATE);
  device.messenger->sendCmdArg(value);
  device.messenger->sendCmdEnd();
}

void rotary_write() {
  int mode = device.messenger->readIntArg();
  switch (mode) {
    case WRITE_DEFAULT:
      setValue(0);
      break;
    case WRITE_ABSOLUTE: {
      setValue(device.messenger->readIntArg());
      break;
    }
    case WRITE_INCREMENT: {
      setValue(value + device.messenger->readIntArg());
      break;
    }
    case WRITE_INCREMENT_DEFAULT:
      setValue(value + 10);
      break;
    case WRITE_DECREMENT: {
      setValue(value - device.messenger->readIntArg());
      break;
    }
    case WRITE_DECREMENT_DEFAULT:
      setValue(value - 10);
      break;
    case WRITE_TOGGLE:
      setValue(value != 255 ? 255 : 0);
      break;
    default:
      break;
  }
}

