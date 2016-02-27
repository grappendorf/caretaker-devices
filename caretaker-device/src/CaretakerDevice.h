/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <EEPROMex.h>
#include <WiFly.h>
#include <CmdMessenger.h>
#include "messages.h"

#ifndef _DEVICE_H
#define _DEVICE_H

// The WiFly serial device
#define WIFLY_SERIAL_HARDWARE
// #define WIFLY_SERIAL_SOFTWARE
#define WIFLY_RXD_PIN 2
#define WIFLY_TXD_PIN 3
#define WIFLY_BAUDRATE 9600

// Define to set a non standard (55555) broadcast port
//#define BROADCAST_PORT 44444

// Define to enable auto configuration code
#define AUTO_CONFIG

// Define to enable debug logging
//#define DEBUG

// The debug serial device
// #define DEBUG_SERIAL_HARDWARE
#define DEBUG_SERIAL_SOFTWARE
#define DEBUG_RXD_PIN 2
#define DEBUG_TXD_PIN 3
#define DEBUG_BAUDRATE 9600

#if (defined WIFLY_SERIAL_SOFTWARE) || (defined DEBUG && defined DEBUG_SERIAL_SOFTWARE)
#iinclude "SoftwareSerial.h"
#endif

typedef struct _DeviceDescriptor {
  const char* type;
  const char* description;
  int ledPin;
  int buttonPin;
  void (*registerMessageHandlers)();
  void (*sendServerRegisterParams)();
  void (*operationalCallback)();
  CmdMessenger* messenger;
} DeviceDescriptor;

void deviceInit(DeviceDescriptor& descriptor);
void deviceUpdate();
bool deviceIsOperational();
void deviceWiflyFlush();
void deviceWiflySleepAfter(int seconds);
void deviceWiflyWakeup();
void deviceWiflyRepl();

#ifdef DEBUG
extern Stream& debug;
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

#define MAKE_STRING_PRE(x) #x
#define MAKE_STRING(x) MAKE_STRING_PRE(x)

#endif
