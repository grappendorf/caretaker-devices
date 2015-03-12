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

/** IO pin numbers */
const uint8_t PIN_LED_RED = 5;
const uint8_t PIN_LED_GREEN = 6;
const uint8_t PIN_LED_BLUE = 9;
const uint8_t PIN_BUTTON = 7;

DeviceDescriptor device;

/** Top switch button */
Bounce button(PIN_BUTTON, 10);

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
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_BLUE, LOW);
  digitalWrite(PIN_BUTTON, HIGH);
  device.type = "DimmerRgb";
  device.description = "RGB Dimmer";
  device.led_pin = PIN_LED_RED;
  device.button_pin = PIN_BUTTON;
  device.register_message_handlers = register_message_handlers;
  device_init(device);
}

/**
 * Main system loop.
 */
void loop() {
  device_update();

  if (device_is_operational()) {
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
  analogWrite(PIN_LED_RED, red);
  analogWrite(PIN_LED_GREEN, green);
  analogWrite(PIN_LED_BLUE, blue);
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
