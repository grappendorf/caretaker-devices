/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

/**
 * The code implementing the zero crossing control of the triac is based
 * on the code of the 'Lichtwecker' projekt by Aike Terjung, published
 * in the Elektor magazin February 2011 on pages 48 ff.
 */

#include <Arduino.h>
#include <Bounce/Bounce.h>
#include <device.h>

/** Digital I/O pin numbers */
const int INFO_LED_PIN = 8;
const int SYS_BUTTON_PIN = 7;
const int PIN_INT0 = 2;
const int PIN_TRIAC = 19;
#define PORT_TRIAC PORTC
#define BIT_TRIAC PC5

/** LED durations */
const uint8_t LED_BLINK_DURATION = 100;
const uint8_t LED_BLINK_DELAY = 50;

/** When to turn of the LED */
unsigned long ledOffMillis = 0;

/** Next time on which the LED can blink again */
unsigned long ledNextMillis = 0;

/** Key debouncer */
Bounce key(SYS_BUTTON_PIN, 10);

/**
 * Maximum interval during which the key must be pressed and released to be
 * detected as clicked
 */
const unsigned long KEY_CLICKED_INTERVAL = 250;

/**
 * Minimum interval during which the key must be pressed to be detected as
 * hold
 */
const unsigned long KEY_HOLD_INTERVAL = 500;

/** Timestamp until the key must be released to be detected as clicked */
unsigned long keyClickedMillis = 1000;

/** Triac trigger duration (1 ms) */
const uint16_t TRIAC_TRIGGER_DURATION = 2000;

/** True if the triac is about to be triggered */
volatile bool triggerTriac = false;

/** Minimum triac trigger delay 0.36 ms */
const uint16_t TRIAC_DELAY_MIN = 720;

/** Maximum triac triggel delay 8.96 ms */
const uint16_t TRIAC_DELAY_MAX = 17920;

/** Actual triac trigger delay */
uint16_t triacDelay = TRIAC_DELAY_MAX;

/** Current brightness value (0..255) */
uint8_t brightness = 0;

/** Brightness fade direction */
int8_t brightnessFadeDelta = -1;

/** During brightness fade: Next Timestamp when to update the brightness */
unsigned long nextBrightnessFadeMillis = 0;

/** Brightness fade update delay */
const unsigned long brightnesFadeDelay = 10;

/** Device configuration */
DeviceDescriptor device;

void register_message_handlers();
void blinkLED();
void calmDownLED();
void configureINT0();
void configureTimer1();
void setBrightness(uint8_t val);
void pwm_read();
void pwm_write();

/**
 * System setup.
 */
void setup() {
  device.type = "Dimmer";
  device.description = "Dimmer";
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.registerMessageHandlers = register_message_handlers;
  deviceInit(device);

  pinMode(INFO_LED_PIN, OUTPUT);
  pinMode(PIN_TRIAC, OUTPUT);
  pinMode(SYS_BUTTON_PIN, INPUT);
  digitalWrite(SYS_BUTTON_PIN, HIGH);
  configureINT0();
  configureTimer1();
}

/**
 * Main execution loop.
 */
void loop() {
  deviceUpdate();

  if (deviceIsOperational()) {
    calmDownLED();

    key.update();
    if (key.fallingEdge()) {
      keyClickedMillis = millis() + KEY_CLICKED_INTERVAL;
    } else if (key.risingEdge() && millis() < keyClickedMillis) {
      if (brightnessFadeDelta == 1) {
        setBrightness(255);
      } else {
        setBrightness(0);
      }
    } else if (key.read() == LOW && key.duration() > KEY_HOLD_INTERVAL) {
      if (millis() > nextBrightnessFadeMillis) {
        setBrightness(brightness + brightnessFadeDelta);
        nextBrightnessFadeMillis = millis() + brightnesFadeDelay;
      }
    }
  }
}

/**
 * Register device specific message handlers.
 */
void register_message_handlers() {
  device.messenger->attach(MSG_PWM_WRITE, pwm_write);
  device.messenger->attach(MSG_PWM_READ, pwm_read);
}

/**
 * Flash the LED.
 */
void blinkLED() {
  if (millis() > ledNextMillis) {
    ledOffMillis = millis() + LED_BLINK_DURATION;
    ledNextMillis = ledOffMillis + LED_BLINK_DELAY;
    digitalWrite(INFO_LED_PIN, HIGH);
  }
}

/**
 * Switch the LED off if the blink duration elapsed.
 */
void calmDownLED() {
  if (millis() > ledOffMillis) {
    digitalWrite(INFO_LED_PIN, LOW);
  }
}

/**
 * Enable timer 1, running with 16/8 = 2MHz.
 * Enable output compare A interrupt.
 */
void configureTimer1() {
  TCCR1A = 0;
  TCCR1B = (1 << CS11);
  OCR1A = 20000;
  TCNT1 = 0;
  TIMSK1 |= (1 << OCIE1A);
}

/**
 * This interrupt implements the zero crossing functionality. It is called
 * two times. First it is called after an INT0 interrupt and triggeres the
 * triac. The compare register A is set in INT0 depending on the value of
 * the brightness variable. The greater this value is, the later the triac
 * will be triggered (resulting in darker light). The compare register is
 * then increased by TRIAC_TRIGGER_DURATION, so after this delay (1 ms)
 * this interrupt is called again. It then switches off the triac gate.
 */
ISR (TIMER1_COMPA_vect) {
  if (triggerTriac) {
    OCR1A += TRIAC_TRIGGER_DURATION;
    PORT_TRIAC |= (1 << BIT_TRIAC);
    triggerTriac = false;
  } else {
    PORT_TRIAC &= ~(1 << BIT_TRIAC);
  }
}

/**
 * Configure the INT0 interrupt on a rising edge. The rising edge is generated on
 * each zero crossing of the 50 Hz AC power line, so the INT0 interrupt is called
 * every 100 ms.
 * This interrupt is disabled by default. Only for brightness values > 0, the
 * INT0 interrupt will be enabled.
 */
void configureINT0() {
  pinMode(PIN_INT0, INPUT);
  digitalWrite(PIN_INT0, HIGH);
  EICRA &= ~(1 << ISC00);
  EICRA |= (1 << ISC01);
}

/**
 * Called on each zero crossing of the 50 Hz AC power line.
 */
ISR (INT0_vect) {
  OCR1A = TCNT1 + triacDelay;
  triggerTriac = true;
  PORT_TRIAC &= ~(1 << BIT_TRIAC);
}

/**
 * Set the brightness.
 *
 * @param val The new brightnes value.
 */
void setBrightness(uint8_t val) {
  blinkLED();
  if (val == 0) {
    cli();
    triacDelay = TRIAC_DELAY_MAX;
    sei();
    EIMSK &= ~(1 << INT0);
  } else {
    cli();
    triacDelay = map(val, 0, 255, TRIAC_DELAY_MAX, TRIAC_DELAY_MIN);
    sei();
    EIMSK |= (1 << INT0);
  }
  brightness = val;
  if (val == 255) {
    brightnessFadeDelta = -1;
  } else if (val == 0) {
    brightnessFadeDelta = 1;
  }
  pwm_read();
}

/**
 * Called when a MSG_PWM_WRITE was received.
 */
void pwm_write() {
  int mode = device.messenger->readIntArg();
  switch (mode) {
    case WRITE_DEFAULT:
      setBrightness(0);
      break;
    case WRITE_ABSOLUTE: {
      int value = device.messenger->readIntArg();
      setBrightness(value);
      break;
    }
    case WRITE_INCREMENT: {
      int value = device.messenger->readIntArg();
      setBrightness(brightness + value);
      break;
    }
    case WRITE_INCREMENT_DEFAULT:
      setBrightness(brightness + 10);
      break;
    case WRITE_DECREMENT: {
      int value = device.messenger->readIntArg();
      setBrightness(brightness - value);
      break;
    }
    case WRITE_DECREMENT_DEFAULT:
      setBrightness(brightness - 10);
      break;
    case WRITE_TOGGLE:
      setBrightness(brightness != 255 ? 255 : 0);
      break;
    default:
      break;
  }
}

/**
 * Called when a MSG_PWM_READ was received.
 */
void pwm_read() {
  device.messenger->sendCmdStart(MSG_PWM_STATE);
  device.messenger->sendCmdArg(brightness);
  device.messenger->sendCmdEnd();
}
