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

// Define to set a non standard (55555) broadcast port
//#define BROADCAST_PORT 44444

// Define to enable auto configuration code
#define AUTO_CONFIG

// Define to enable debug logging
//#define DEBUG

typedef struct _DeviceDescriptor {
  const char* type;
  const char* description;
  int ledPin;
  int buttonPin;
  void (*registerMessageHandlers) ();
  void (*sendServerRegisterParams) ();
  CmdMessenger* messenger;
} DeviceDescriptor;

void deviceInit(DeviceDescriptor& descriptor);
void deviceUpdate();
bool deviceIsOperational();
void deviceWiflyFlush();

#ifdef DEBUG
#include <SoftwareSerial/SoftwareSerial.h>
extern SoftwareSerial debug;
#define DEBUG_PRINT(s) debug.print(s);
#define DEBUG_PRINTLN(s) debug.println(s);
#define DEBUG_PRINTLN_STATE(s) if (state != debugLastState) { \
  debug.print(F("STATE_")); debug.println(s); debugLastState = state; }
#define DEBUG_DUMP_CONFIG_VALUES() dumpConfigValues();
#else
#define DEBUG_PRINT(s)
#define DEBUG_PRINTLN(s)
#define DEBUG_PRINTLN_STATE(s)
#define DEBUG_DUMP_CONFIG_VALUES()
#endif

#endif
