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
#include "Bounce.h"

const uint8_t LED_RED_PIN = 9;
const uint8_t LED_GREEN_PIN = 11;
const uint8_t LED_BLUE_PIN = 10;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t INFO_LED_PIN = 13;
const uint8_t MANUAL_BUTTON_PIN = SYS_BUTTON_PIN;

DeviceDescriptor device;

Bounce button(MANUAL_BUTTON_PIN, 10);

uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;

void setup() {
  device.type = "DimmerRgb";
  device.description = "RGB LED demo shield";
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.registerMessageHandlers = register_message_handlers;
  deviceInit(device);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
  pinMode(SYS_BUTTON_PIN, INPUT);
  digitalWrite(SYS_BUTTON_PIN, HIGH);
}

void loop() {
  deviceUpdate();

  if (deviceIsOperational()) {
    button.update();
    if (button.fallingEdge()) {
      toggle();
    }
  }
}

void register_message_handlers() {
  device.messenger->attach(MSG_RGB_WRITE, rgb_write);
  device.messenger->attach(MSG_RGB_READ, rgb_read);
}

void rgb(uint8_t r, uint8_t g, uint8_t b) {
  red = r;
  green = g;
  blue = b;
  analogWrite(LED_RED_PIN, red);
  analogWrite(LED_GREEN_PIN, green);
  analogWrite(LED_BLUE_PIN, blue);
  rgb_read();
}

void toggle() {
  if (red > 0 || green > 0 || blue > 0) {
    rgb(0, 0, 0);
  } else {
    rgb(255, 255, 255);
  }
}

void rgb_write() {
  int mode = device.messenger->readIntArg();
  switch (mode) {
    case WRITE_DEFAULT:
      rgb(0, 0, 0);
      break;
    case WRITE_ABSOLUTE: {
      int red = device.messenger->readIntArg();
      int green = device.messenger->readIntArg();
      int blue = device.messenger->readIntArg();
      rgb(red, green, blue);
      break;
    }
    case WRITE_INCREMENT: {
      int red = device.messenger->readIntArg();
      int green = device.messenger->readIntArg();
      int blue = device.messenger->readIntArg();
      rgb(red, green, blue);
      break;
    }
    case WRITE_INCREMENT_DEFAULT:
      rgb(red + 32, green + 32, blue + 32);
      break;
    case WRITE_DECREMENT: {
      int red = device.messenger->readIntArg();
      int green = device.messenger->readIntArg();
      int blue = device.messenger->readIntArg();
      rgb(red, green, blue);
      break;
    }
    case WRITE_DECREMENT_DEFAULT:
      rgb(red + 32, green + 32, blue + 32);
      break;
    case WRITE_TOGGLE:
      toggle();
      break;
    default:
      break;
  }
}

void rgb_read() {
  device.messenger->sendCmdStart(MSG_RGB_STATE);
  device.messenger->sendCmdArg(red);
  device.messenger->sendCmdArg(green);
  device.messenger->sendCmdArg(blue);
  device.messenger->sendCmdEnd();
}

