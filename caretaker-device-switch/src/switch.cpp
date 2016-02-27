/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Arduino.h>
#include <Bounce.h>
#include <CaretakerDevice.h>

#define BOARD_TYPE_SWITCH
//#define BOARD_TYPE_DEMO_SHIELD

// Configuration for switch board
#ifdef BOARD_TYPE_SWITCH
#define DEVICE_DESCRIPTION "Single port switch"
const uint8_t INFO_LED_PIN = 13;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t SWITCH_PIN = 9;
const uint8_t MAUNAL_BUTTON_PIN = 8;
#endif

// Pin assignments for the demo shield board
#ifdef BOARD_TYPE_DEMO_SHIELD
#define DEVICE_DESCRIPTION "Single switch demo shield"
const uint8_t INFO_LED_PIN = 13;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t SWITCH_PIN = 9;
const uint8_t MAUNAL_BUTTON_PIN = 8;
#endif

const uint8_t NUM_SWITCHES = 1;

DeviceDescriptor device;

Bounce button(MAUNAL_BUTTON_PIN, 5);

void send_server_register_params();
void register_message_handlers();
void switch_read();
void switch_write();

/**
 * System setup.
 */
void setup() {
  device.type = "Switch";
  device.description = DEVICE_DESCRIPTION;
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.registerMessageHandlers = register_message_handlers;
  device.sendServerRegisterParams = send_server_register_params;
  deviceInit(device);

  pinMode(MAUNAL_BUTTON_PIN, INPUT);
  digitalWrite(MAUNAL_BUTTON_PIN, HIGH);

  pinMode(SWITCH_PIN, OUTPUT);
}

/**
 * Main execution loop.
 */
void loop() {
  deviceUpdate();
  if (deviceIsOperational()) {
    button.update();
    if (button.fallingEdge()) {
      digitalWrite(SWITCH_PIN, digitalRead(SWITCH_PIN) == HIGH ? LOW : HIGH);
      switch_read();
    }
  }
}

/**
 * Send additional parameter with the MSG_REGISTER_REQUEST.
 *
 * @param messenger
 */
void send_server_register_params() {
  device.messenger->sendCmdArg(NUM_SWITCHES);
}

/**
 * Register device specific message handlers.
 */
void register_message_handlers() {
  device.messenger->attach(MSG_SWITCH_WRITE, switch_write);
  device.messenger->attach(MSG_SWITCH_READ, switch_read);
}

/**
 * Called when a MSG_SWITCH_WRITE was received.
 */
void switch_read() {
  device.messenger->sendCmdStart(MSG_SWITCH_STATE);
  device.messenger->sendCmdArg(0);
  device.messenger->sendCmdArg(digitalRead(SWITCH_PIN));
  device.messenger->sendCmdEnd();
}

/**
 * Called when a MSG_SWITCH_WRITE was received.
 */
void switch_write() {
  device.messenger->next(); // Ignore switch number
  int mode = device.messenger->readIntArg();
  switch (mode) {
    case WRITE_DEFAULT:
      digitalWrite(SWITCH_PIN, LOW);
      break;
    case WRITE_ABSOLUTE:
      digitalWrite(SWITCH_PIN, device.messenger->readIntArg() != 0 ? HIGH : LOW);
      break;
    case WRITE_INCREMENT:
      device.messenger->next(); // Ignore increment value
      digitalWrite(SWITCH_PIN, HIGH);
      break;
    case WRITE_INCREMENT_DEFAULT:
      digitalWrite(SWITCH_PIN, HIGH);
      break;
    case WRITE_DECREMENT:
      device.messenger->next(); // Ignore decrement value
      digitalWrite(SWITCH_PIN, LOW);
      break;
    case WRITE_DECREMENT_DEFAULT:
      digitalWrite(SWITCH_PIN, LOW);
      break;
    case WRITE_TOGGLE:
      digitalWrite(SWITCH_PIN, digitalRead(SWITCH_PIN) == LOW ? HIGH : LOW);
      break;
    default:
      break;
  }
  switch_read();
}
