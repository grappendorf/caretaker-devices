/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

/**
 * This code uses 11 pin change interrupt pins. So ensure that the value of
 * MAX_PIN_CHANGE_PINS is at least set to 11 in PinChangeInt.h.
 *
 * This code doesn't compile if DEBUG is enabled in device.h.
 * SoftwareSerial conflicts with PinChangeInt because SoftwareSerial also
 * uses pin change interrupts.
 */

/**
 * Fuse bits for ATmega328P:
 *
 * Low:      0xFF
 * High:     0xDF
 * Extended: 0xFD
 */

#define DEVICE_TYPE "RemoteControl"

// #define BOARD_TYPE_REMOTE_CONTROL
#define BOARD_TYPE_DEMO_SHIELD

// Configuration for the remote control board
#ifdef BOARD_TYPE_REMOTE_CONTROL
#define DEVICE_DESCRIPTION "10 button Remote Control"
const uint8_t LED_RED_PIN = 19;
const uint8_t LED_GREEN_PIN = 18;
const uint8_t LED_YELLOW_PIN = 17;
const uint8_t BUTTON_01_PIN = 3;
const uint8_t BUTTON_02_PIN = 4;
const uint8_t BUTTON_03_PIN = 5;
const uint8_t BUTTON_04_PIN = 6;
const uint8_t BUTTON_05_PIN = 7;
const uint8_t BUTTON_06_PIN = 16;
const uint8_t BUTTON_07_PIN = 15;
const uint8_t BUTTON_08_PIN = 14;
const uint8_t BUTTON_09_PIN = 10;
const uint8_t BUTTON_10_PIN = 9;
const uint8_t LOW_BAT_PIN = 8;
const uint8_t WIFLY_SLEEP_PIN = 2;
const uint8_t INFO_LED_PIN = LED_YELLOW_PIN;
const uint8_t SYS_BUTTON_PIN = BUTTON_01_PIN;
const uint8_t AWAKE_LED_PIN = LED_GREEN_PIN;
const uint8_t LOW_POWER_LED_PIN = LED_RED_PIN;
const uint8_t NUM_BUTTONS = 10;
#define ENABLE_LOW_POWER
#endif

// Configuration the demo shield board
#ifdef BOARD_TYPE_DEMO_SHIELD
#define DEVICE_DESCRIPTION "4 buttons demo shield"
const uint8_t BUTTON_01_PIN = 11;
const uint8_t BUTTON_02_PIN = 9;
const uint8_t BUTTON_03_PIN = 10;
const uint8_t BUTTON_04_PIN = 8;
const uint8_t BUTTON_05_PIN = 0;
const uint8_t BUTTON_06_PIN = 0;
const uint8_t BUTTON_07_PIN = 0;
const uint8_t BUTTON_08_PIN = 0;
const uint8_t BUTTON_09_PIN = 0;
const uint8_t BUTTON_10_PIN = 0;
const uint8_t INFO_LED_PIN = 13;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t NUM_BUTTONS = 4;
#endif
