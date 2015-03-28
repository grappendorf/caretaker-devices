/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Arduino.h>
#include <Bounce/Bounce.h>
#include <device.h>

//#define BOARD_TYPE_SPOEKA
#define BOARD_TYPE_DEMO_SHIELD

// Configuration for the Spoeka board
#ifdef BOARD_TYPE_SPOEKA
#define DEVICE_NAME "Caretaker-RGBDimmer"
#define DEVICE_DESCRIPTION "Dimmable RGB Light"
const uint8_t LED_RED_PIN = 5;
const uint8_t LED_GREEN_PIN = 6;
const uint8_t LED_BLUE_PIN = 9;
const uint8_t SYS_BUTTON_PIN = 7;
const uint8_t INFO_LED_PIN = LED_RED_PIN;
const uint8_t MANUAL_BUTTON_PIN = SYS_BUTTON_PIN;
#endif

// Configuration for the demo shield board
#ifdef BOARD_TYPE_DEMO_SHIELD
#define DEVICE_NAME "DemoShield RGB"
#define DEVICE_DESCRIPTION "Arduino Demo Shield with RGB LED"
const uint8_t LED_RED_PIN = 9;
const uint8_t LED_GREEN_PIN = 11;
const uint8_t LED_BLUE_PIN = 10;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t INFO_LED_PIN = 13;
const uint8_t MANUAL_BUTTON_PIN = SYS_BUTTON_PIN;
#endif

DeviceDescriptor device;

/** Top switch button */
Bounce button(MANUAL_BUTTON_PIN, 10);

/** Current RGB values */
uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;

void register_message_handlers();
void rgb(uint8_t r, uint8_t g, uint8_t b);
void toggle();
void rgb_write();
void rgb_read();

/**
 * Main system setup.
 */
void setup() {
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
  pinMode(SYS_BUTTON_PIN, INPUT);
  digitalWrite(SYS_BUTTON_PIN, HIGH);
  device.type = DEVICE_NAME;
  device.description = DEVICE_DESCRIPTION;
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.registerMessageHandlers = register_message_handlers;
  deviceInit(device);
}

/**
 * Main system loop.
 */
void loop() {
  deviceUpdate();

  if (deviceIsOperational()) {
    button.update();
    if (button.fallingEdge()) {
      toggle();
    }
  }
}

/**
 * Register device specific message handlers.
 */
void register_message_handlers() {
  device.messenger->attach(MSG_RGB_WRITE, rgb_write);
  device.messenger->attach(MSG_RGB_READ, rgb_read);
}

/**
 * Set the RGB values.
 */
void rgb(uint8_t r, uint8_t g, uint8_t b) {
  red = r;
  green = g;
  blue = b;
  analogWrite(LED_RED_PIN, red);
  analogWrite(LED_GREEN_PIN, green);
  analogWrite(LED_BLUE_PIN, blue);
  rgb_read();
}

/**
 * Toggle between on (white) and off.
 */
void toggle() {
  if (red > 0 || green > 0 || blue > 0) {
    rgb(0, 0, 0);
  } else {
    rgb(255, 255, 255);
  }
}

/**
 * Called when a MSG_RGB_WRITE was received.
 */
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

/**
 * Called when a MSG_RGB_READ was received.
 */
void rgb_read() {
  device.messenger->sendCmdStart(MSG_RGB_STATE);
  device.messenger->sendCmdArg(red);
  device.messenger->sendCmdArg(green);
  device.messenger->sendCmdArg(blue);
  device.messenger->sendCmdEnd();
}
