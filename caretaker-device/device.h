/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <messages.h>
#include <CmdMessenger/CmdMessenger.h>

#ifndef _DEVICE_H
#define _DEVICE_H

typedef struct _DeviceDescriptor {
  const char* type;
  const char* description;
  int led_pin;
  int button_pin;
  void (*register_message_handlers) ();
  void (*send_server_register_params) ();
  CmdMessenger* messenger;
} DeviceDescriptor;

void device_init(DeviceDescriptor& descriptor);
void device_update();
bool device_is_operational();

//#define DEBUG

#ifdef DEBUG
#include <SoftwareSerial/SoftwareSerial.h>
extern SoftwareSerial debug;
#define DEBUG_PRINT(s) debug.print(s);
#define DEBUG_PRINTLN(s) debug.println(s);
#define DEBUG_PRINTLN_STATE(s) if (state != debug_last_state) { \
  debug.print(F("STATE_")); debug.println(s); debug_last_state = state; }
#define DEBUG_DUMP_CONFIG_VALUES() dump_config_values();
#else
#define DEBUG_PRINT(s)
#define DEBUG_PRINTLN(s)
#define DEBUG_PRINTLN_STATE(s)
#define DEBUG_DUMP_CONFIG_VALUES()
#endif

#endif
