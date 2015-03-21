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
 */

/**
 * Fuse bits for ATmega328P:
 *
 * Low:      0xFF
 * High:     0xDF
 * Extended: 0xFD
 */

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <PinChangeInt/PinChangeInt.h>
#include <device.h>
#include <Bounce/Bounce.h>

/** IO Pins */
const uint8_t WIFLY_SLEEP = 2;
const uint8_t LED_RED = 19;
const uint8_t LED_GREEN = 18;
const uint8_t LED_YELLOW = 17;
const uint8_t BUTTON_01 = 3;
const uint8_t BUTTON_02 = 4;
const uint8_t BUTTON_03 = 5;
const uint8_t BUTTON_04 = 6;
const uint8_t BUTTON_05 = 7;
const uint8_t BUTTON_06 = 16;
const uint8_t BUTTON_07 = 15;
const uint8_t BUTTON_08 = 14;
const uint8_t BUTTON_09 = 10;
const uint8_t BUTTON_10 = 9;
const uint8_t LOW_BAT = 8;

/** Some LED constants */
enum Led {
  NONE = 0, YELLOW = 1, GREEN = 2, RED = 4, ALL = 7
};

/** Buttons */
uint8_t buttonPins[] = { BUTTON_01, BUTTON_02, BUTTON_03, BUTTON_04, BUTTON_05, BUTTON_06, BUTTON_07, BUTTON_08,
    BUTTON_09, BUTTON_10 };

Bounce buttons[] = { Bounce(BUTTON_01, 5), Bounce(BUTTON_02, 5), Bounce(BUTTON_03, 5), Bounce(BUTTON_04, 5), Bounce(
    BUTTON_05, 5), Bounce(BUTTON_06, 5), Bounce(BUTTON_07, 5), Bounce(BUTTON_08, 5), Bounce(BUTTON_09, 5), Bounce(
    BUTTON_10, 5) };

const uint8_t NUM_BUTTONS = sizeof(buttonPins) / sizeof(buttonPins[0]);


/** States of the device mode state engine */
enum State {
  POWER_UP, NORMAL_MODE, LOW_BATTERY
};

/** Current device mode state */
State state = POWER_UP;

/** How long the device stays awake */
const unsigned long AWAKE_TIMEOUT_MILLIS = 20 * 1000;

/** When to enter the sleep mode */
unsigned long enterSleepModeMillis;

/** BLink duration of the yellow LED */
const unsigned long YELLOW_LED_BLINK_MILLIS = 250;

/** When to switch off the yellow LED */
unsigned long yellowLedOffMillis;

/** Device information. */
DeviceDescriptor device;

void send_server_register_params();
bool isLowBattery();
void buttonChanged();
void deviceOperationalCallback();

/**
 * Main system setup.
 */
void setup() {
  device.type = "RemoteControl";
  device.description = "Remote Control";
  device.ledPin = 13;
  device.buttonPin = 4;
  device.sendServerRegisterParams = send_server_register_params;
  device.operationalCallback = deviceOperationalCallback;
  deviceInit(device);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  pinMode(BUTTON_01, INPUT);
  digitalWrite(BUTTON_01, HIGH);
  for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
    pinMode(buttonPins[i], INPUT);
    digitalWrite(buttonPins[i], HIGH);
    PCattachInterrupt(buttonPins[i], buttonChanged, CHANGE);
  }
  pinMode(WIFLY_SLEEP, INPUT);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_adc_disable ();
  power_spi_disable ();
  power_twi_disable ();
  power_timer1_disable ();
  power_timer2_disable ();
}

/**
 * Main system loop.
 */
void loop() {
  if (isLowBattery()) {
    state = LOW_BATTERY;
  }

  switch (state) {
    case POWER_UP:
      state = NORMAL_MODE;
      break;

    case LOW_BATTERY:
      digitalWrite(LED_RED, HIGH);
      break;

    case NORMAL_MODE:
      deviceUpdate();

      if (deviceIsOperational()) {
        if (millis() > enterSleepModeMillis) {
          sleep_enable()
          ;
          sleep_mode()
          ;
          sleep_disable()
          ;
        }

        digitalWrite(LED_GREEN, digitalRead(WIFLY_SLEEP));

        if (millis() > yellowLedOffMillis) {
          digitalWrite(LED_YELLOW, LOW);
        }

        for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
          buttons[i].update();

          if (buttons[i].fallingEdge() || buttons[i].risingEdge()) {
            digitalWrite(LED_YELLOW, HIGH);
            yellowLedOffMillis = millis() + YELLOW_LED_BLINK_MILLIS;
            int state = buttons[i].read();
            bool wiflySleeping = digitalRead(WIFLY_SLEEP) == LOW;
            if (wiflySleeping) {
              deviceWiflyWakeup();
            }
            device.messenger->sendCmdStart(MSG_BUTTON_STATE);
            device.messenger->sendCmdArg(i);
            device.messenger->sendCmdArg(state == LOW ? 1 : 0);
            device.messenger->sendCmdEnd();
            if (wiflySleeping) {
              delay(100);
            }
          }
        }
      }

      break;
  }
}

/**
 * Send additional parameter with the MSG_REGISTER_REQUEST.
 *
 * @param messenger
 */
void send_server_register_params() {
  device.messenger->sendCmdArg(NUM_BUTTONS);
}

/**
 * Check for low battery state and inform the user.
 */
bool isLowBattery() {
  return digitalRead(LOW_BAT) == LOW;
}

/**
 * This is the interrupt routine, triggered by the pin change interrupt.
 */
void buttonChanged() {
  enterSleepModeMillis = millis() + AWAKE_TIMEOUT_MILLIS;
}

/**
 * Called when the device enters the operational state.
 */
void deviceOperationalCallback() {
  deviceWiflySleepAfter(10);
  enterSleepModeMillis = millis() + AWAKE_TIMEOUT_MILLIS;
}
