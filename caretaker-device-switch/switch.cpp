/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Arduino.h>
#include <device.h>

#define NUM_SWITCHES 1

DeviceDescriptor device;

void send_server_register_params();
void register_message_handlers();
void switch_read();
void switch_write();

/**
 * System setup.
 */
void setup() {
  device.type = "Switch";
  device.description = "Single Port Switch";
  device.led_pin = 13;
  device.button_pin = 4;
  device.register_message_handlers = register_message_handlers;
  device.send_server_register_params = send_server_register_params;
  device_init(device);
}

/**
 * Main execution loop.
 */
void loop() {
  device_update();
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
  device.messenger->sendCmdArg(digitalRead(device.led_pin));
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
      digitalWrite(device.led_pin, LOW);
      break;
    case WRITE_ABSOLUTE:
      digitalWrite(device.led_pin, device.messenger->readIntArg() != 0 ? HIGH : LOW);
      break;
    case WRITE_INCREMENT:
      device.messenger->next(); // Ignore increment value
      digitalWrite(device.led_pin, HIGH);
      break;
    case WRITE_INCREMENT_DEFAULT:
      digitalWrite(device.led_pin, HIGH);
      break;
    case WRITE_DECREMENT:
      device.messenger->next(); // Ignore decrement value
      digitalWrite(device.led_pin, LOW);
      break;
    case WRITE_DECREMENT_DEFAULT:
      digitalWrite(device.led_pin, LOW);
      break;
    case WRITE_TOGGLE:
      digitalWrite(device.led_pin, digitalRead(device.led_pin) == LOW ? HIGH : LOW);
      break;
    default:
      break;
  }
  switch_read();
}
