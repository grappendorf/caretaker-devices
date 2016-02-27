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

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <PinChangeInt.h>
#include <Bounce.h>
#include <CaretakerDevice.h>

#define BOARD_TYPE_REMOTE_CONTROL
//#define BOARD_TYPE_DEMO_SHIELD

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

/** Buttons */
uint8_t buttonPins[] = { BUTTON_01_PIN, BUTTON_02_PIN, BUTTON_03_PIN, BUTTON_04_PIN, BUTTON_05_PIN, BUTTON_06_PIN, BUTTON_07_PIN, BUTTON_08_PIN,
    BUTTON_09_PIN, BUTTON_10_PIN };

Bounce buttons[] = { Bounce(BUTTON_01_PIN, 5), Bounce(BUTTON_02_PIN, 5), Bounce(BUTTON_03_PIN, 5), Bounce(BUTTON_04_PIN, 5), Bounce(
    BUTTON_05_PIN, 5), Bounce(BUTTON_06_PIN, 5), Bounce(BUTTON_07_PIN, 5), Bounce(BUTTON_08_PIN, 5), Bounce(BUTTON_09_PIN, 5), Bounce(
    BUTTON_10_PIN, 5) };

/** BLink duration of the info LED */
const unsigned long INFO_LED_BLINK_MILLIS = 250;

/** When to switch off the info LED */
unsigned long infoLedOffMillis;

/** Device information. */
DeviceDescriptor device;

void send_server_register_params();
void deviceOperationalCallback();

#ifdef ENABLE_LOW_POWER
/** How long the device stays awake */
const unsigned long AWAKE_TIMEOUT_SECONDS = 3;

/** When to enter the sleep mode */
unsigned long enterSleepModeMillis;

bool isLowBattery();
void buttonChanged();
#endif

/**
 * Main system setup.
 */
void setup() {
  device.type = "RemoteControl";
  device.description = DEVICE_DESCRIPTION;
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.sendServerRegisterParams = send_server_register_params;
  device.operationalCallback = deviceOperationalCallback;
  deviceInit(device);

  pinMode(INFO_LED_PIN, OUTPUT);
  digitalWrite(INFO_LED_PIN, LOW);
  for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
    pinMode(buttonPins[i], INPUT);
    digitalWrite(buttonPins[i], HIGH);
#ifdef ENABLE_LOW_POWER
    PCattachInterrupt(buttonPins[i], buttonChanged, CHANGE);
#endif
  }

#ifdef ENABLE_LOW_POWER
  pinMode(AWAKE_LED_PIN, OUTPUT);
  digitalWrite(AWAKE_LED_PIN, LOW);
  pinMode(LOW_POWER_LED_PIN, OUTPUT);
  digitalWrite(LOW_POWER_LED_PIN, LOW);
  pinMode(WIFLY_SLEEP_PIN, INPUT);
  pinMode(LOW_BAT_PIN, INPUT);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_adc_disable ();
  power_spi_disable ();
  power_twi_disable ();
  power_timer1_disable ();
  power_timer2_disable ();
#endif
}

/**
 * Main system loop.
 */
void loop() {
#ifdef ENABLE_LOW_POWER
  digitalWrite(LOW_POWER_LED_PIN, isLowBattery() ? HIGH : LOW);
#endif

  deviceUpdate();

  if (deviceIsOperational()) {
#ifdef ENABLE_LOW_POWER
    if (millis() > enterSleepModeMillis) {
      sleep_enable()
      ;
      sleep_mode()
      ;
      sleep_disable()
      ;
    }
    digitalWrite(AWAKE_LED_PIN, digitalRead(WIFLY_SLEEP_PIN));
#endif

    if (millis() > infoLedOffMillis) {
      digitalWrite(INFO_LED_PIN, LOW);
    }

    for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
      buttons[i].update();

      if (buttons[i].fallingEdge() || buttons[i].risingEdge()) {

        digitalWrite(INFO_LED_PIN, HIGH);
        infoLedOffMillis = millis() + INFO_LED_BLINK_MILLIS;

        int state = buttons[i].read();

#ifdef ENABLE_LOW_POWER
        bool wiflySleeping = digitalRead(WIFLY_SLEEP_PIN) == LOW;
        if (wiflySleeping) {
          deviceWiflyWakeup();
        }
#endif

        device.messenger->sendCmdStart(MSG_BUTTON_STATE);
        device.messenger->sendCmdArg(i);
        device.messenger->sendCmdArg(state == LOW ? 1 : 0);
        device.messenger->sendCmdEnd();

#ifdef ENABLE_LOW_POWER
        if (wiflySleeping) {
          delay(100);
        }
#endif
      }
    }
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
#ifdef ENABLE_LOW_POWER
bool isLowBattery() {
  return digitalRead(LOW_BAT_PIN) == LOW;
}
#endif

/**
 * This is the interrupt routine, triggered by the pin change interrupt.
 */
#ifdef ENABLE_LOW_POWER
void buttonChanged() {
  enterSleepModeMillis = millis() + (AWAKE_TIMEOUT_SECONDS + 2) * 1000;
}
#endif

/**
 * Called when the device enters the operational state.
 */
void deviceOperationalCallback() {
#ifdef ENABLE_LOW_POWER
  deviceWiflySleepAfter(AWAKE_TIMEOUT_SECONDS);
  enterSleepModeMillis = millis() + (AWAKE_TIMEOUT_SECONDS + 2) * 1000;
#endif
}
