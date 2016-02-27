/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Arduino.h>
#include <CaretakerDevice.h>

/** Digital I/O pin numbers */
const uint8_t PIN_SWITCH0 = 8;
const uint8_t PIN_SWITCH1 = 6;
const uint8_t PIN_SWITCH2 = 4;
const uint8_t PIN_SWITCH3 = 2;
const uint8_t PIN_SWITCH4 = 9;
const uint8_t PIN_SWITCH5 = 7;
const uint8_t PIN_SWITCH6 = 5;
const uint8_t PIN_SWITCH7 = 3;
const uint8_t PIN_BUZZER = 15;

/** Switch pins by number */
const uint8_t switchPins[] = { PIN_SWITCH0, PIN_SWITCH1, PIN_SWITCH2, PIN_SWITCH3, PIN_SWITCH4, PIN_SWITCH5,
		PIN_SWITCH6, PIN_SWITCH7 };

/** Total number of switch pins */
const uint8_t NUM_SWITCH_PINS = sizeof switchPins / sizeof switchPins[0];

/** Beep duration in milli seconds */
const int BEEP_DURATION = 100;

/** When to switch off the buzzer (milli second timestamp) */
unsigned long beepCalmDownTime = 0;

/** Device information */
DeviceDescriptor device;

void beep(int duration);
void calmDownBeep();
void sendServerRegisterParams();
void registerMessageHandlers();
void sendSwitchState(int switchNum);
void switchRead();
void switchWrite();

/**
 * System setup.
 */
void setup()
{
  for (uint8_t i = 0; i < 8; ++i)
  {
    pinMode(switchPins[i], OUTPUT);
  }
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_BUZZER, HIGH);
  delay(50);
  digitalWrite(PIN_BUZZER, LOW);

  device.type = "Switch";
  device.description = "8-Port Switch";
  device.ledPin = 0;
  device.buttonPin = 0;
  device.registerMessageHandlers = registerMessageHandlers;
  device.sendServerRegisterParams = sendServerRegisterParams;
  deviceInit(device);
}

/**
 * Main execution loop.
 */
void loop()
{
  calmDownBeep();
  deviceUpdate();
}

/**
 * Beep the buzzer.
 *
 * @param duration Beep duration in milli seconds
 */
void beep()
{
	digitalWrite(PIN_BUZZER, HIGH);
	beepCalmDownTime = millis() + BEEP_DURATION;
	if (beepCalmDownTime == 0)
	{
		beepCalmDownTime = 1;
	}
}

/**
 * Eventually switch of the buzzer (if it's currently beeping and the
 * beep duration has elapsed.
 */
void calmDownBeep()
{
	if (beepCalmDownTime != 0 && millis() > beepCalmDownTime)
	{
		digitalWrite(PIN_BUZZER, LOW);
		beepCalmDownTime = 0;
	}
}

/**
 * Send additional parameter with the MSG_REGISTER_REQUEST.
 *
 * @param messenger
 */
void sendServerRegisterParams() {
  device.messenger->sendCmdArg(NUM_SWITCH_PINS);
}

/**
 * Register device specific message handlers.
 */
void registerMessageHandlers() {
  device.messenger->attach(MSG_SWITCH_WRITE, switchWrite);
  device.messenger->attach(MSG_SWITCH_READ, switchRead);
}

/**
 * Send the state of the specified switch to the server.
 *
 * @param switchNum The switch number
 */
void sendSwitchState(int switchNum) {
  uint8_t switchPin = switchPins[switchNum];
  device.messenger->sendCmdStart(MSG_SWITCH_STATE);
  device.messenger->sendCmdArg(switchNum);
  device.messenger->sendCmdArg(digitalRead(switchPin) == HIGH ? 1 : 0);
  device.messenger->sendCmdEnd();
}

/**
 * Called when a MSG_SWITCH_READ was received.
 */
void switchRead() {
  int switchNum = device.messenger->readIntArg();
  if (switchNum < 0 || switchNum > 7) {
    return;
  }
  sendSwitchState(switchNum);
}

/**
 * Called when a MSG_SWITCH_WRITE was received.
 */
void switchWrite() {
  int switchNum = device.messenger->readIntArg();
  if (switchNum < 0 || switchNum > 7) {
    return;
  }
  uint8_t switchPin = switchPins[switchNum];
  uint8_t mode = device.messenger->readIntArg();
  switch (mode) {
    case WRITE_DEFAULT:
      digitalWrite(switchPin, LOW);
      break;
    case WRITE_ABSOLUTE:
      digitalWrite(switchPin, device.messenger->readIntArg() != 0 ? HIGH : LOW);
      break;
    case WRITE_INCREMENT:
      device.messenger->next(); // Ignore increment value
      digitalWrite(switchPin, HIGH);
      break;
    case WRITE_INCREMENT_DEFAULT:
      digitalWrite(switchPin, HIGH);
      break;
    case WRITE_DECREMENT:
      device.messenger->next(); // Ignore decrement value
      digitalWrite(switchPin, LOW);
      break;
    case WRITE_DECREMENT_DEFAULT:
      digitalWrite(switchPin, LOW);
      break;
    case WRITE_TOGGLE:
      digitalWrite(switchPin, digitalRead(switchPin) == LOW ? HIGH : LOW);
      break;
    default:
      break;
  }
  beep();
  sendSwitchState(switchNum);
}
