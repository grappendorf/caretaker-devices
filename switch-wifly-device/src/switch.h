/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#ifndef SWITCH_H
#define SWITCH_H

#define DEVICE_TYPE "Switch"

// #define BOARD_TYPE_SWITCH
#define BOARD_TYPE_DEMO_SHIELD

// Configuration for switch board
#ifdef BOARD_TYPE_SWITCH
#define DEVICE_DESCRIPTION "Single port switch"
const uint8_t INFO_LED_PIN = 13;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t SWITCH_PIN = 9;
const uint8_t MANUAL_BUTTON_PIN = 8;
#endif

// Pin assignments for the demo shield board
#ifdef BOARD_TYPE_DEMO_SHIELD
#define DEVICE_DESCRIPTION "Single switch demo shield"
const uint8_t INFO_LED_PIN = 13;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t SWITCH_PIN = 9;
const uint8_t MANUAL_BUTTON_PIN = 8;
#endif

const uint8_t NUM_SWITCHES = 1;

#endif // SWITCH_H
