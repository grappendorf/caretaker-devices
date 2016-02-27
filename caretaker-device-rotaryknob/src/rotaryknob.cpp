/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Arduino.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <MultiMap.h>
#include <CaretakerDevice.h>

#define BOARD_TYPE_ROTARYKNOB
//#define BOARD_TYPE_DEMO_SHIELD

// Configuration for the Rotary Knob wall device
#ifdef BOARD_TYPE_ROTARYKNOB
#define DEVICE_DESCRIPTION "Rotary Knob"
const uint8_t ENCODER_A_PIN = 9;
const uint8_t ENCODER_B_PIN = 10;
const uint8_t ENCODER_BUTTON_PIN = 8;
const uint8_t VALUE_LED_PIN = 11;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t INFO_LED_PIN = 13;
#endif

// Configuration for the demo shield board
#ifdef BOARD_TYPE_DEMO_SHIELD
#define DEVICE_DESCRIPTION "Rotary knob demo shield"
const uint8_t ENCODER_A_PIN = 11;
const uint8_t ENCODER_B_PIN = 10;
const uint8_t ENCODER_BUTTON_PIN = 8;
const uint8_t VALUE_LED_PIN = 9;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t INFO_LED_PIN = 13;
#endif

const int MINIMUM_VALUE = 0;
const int MAXIMUM_VALUE = 255;
const int DELTA_FACTOR = 4;

DeviceDescriptor device;

/** Rotary knob */
ClickEncoder encoder(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BUTTON_PIN);
int value, lastValue;

/** LED brightness value lookup table */
int ledValueIn[] = { 0, 50, 100, 150, 200, 255 };
int ledValueOut[] = { 0, 20, 40, 100, 200, 254 };

void timerIsr();
void setValue(int value);
void register_message_handlers();
void rotary_read();
void rotary_write();

/**
 * Main system setup.
 */
void setup() {
  device.type = "RotaryKnob";
  device.description = DEVICE_DESCRIPTION;
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

/**
 * Main system loop.
 */
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

/**
 * Called every ms.
 */
void timerIsr() {
  encoder.service();
}

/**
 * Set a new value and eventually send a MSG_ROTARY_STATE.
 *
 * @param newValue The new value to set.
 */
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

/**
 * Register device specific message handlers.
 */
void register_message_handlers() {
  device.messenger->attach(MSG_ROTARY_WRITE, rotary_write);
  device.messenger->attach(MSG_ROTARY_READ, rotary_read);
}

/**
 * Called when a MSG_ROTARY_READ was received.
 */
void rotary_read() {
  device.messenger->sendCmdStart(MSG_ROTARY_STATE);
  device.messenger->sendCmdArg(value);
  device.messenger->sendCmdEnd();
}

/**
 * Called when a MSG_ROTARY_WRITE was received.
 */
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
