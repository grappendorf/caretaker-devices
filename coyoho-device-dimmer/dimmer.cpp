/**
 * This file is part of the CoYoHo Control Your Home System.
 *
 * Copyright 2011-2012 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * The code implementing the zero crossing control of the triac is based
 * on the code of the 'Lichtwecker' projekt by Aike Terjung, published
 * in the Elektor magazin February 2011 on pages 48 ff.
 */

#include <Arduino.h>
#include <Bounce/Bounce.h>
#include <XXBee/XXBee.h>
#include <CoYoHoMessages.h>
#include <CoYoHoListenerManager.h>
#include <avr/wdt.h>

/** Digital I/O pin numbers */
const int PIN_LED = 8;
const int PIN_INT0 = 2;
const int PIN_KEY = 7;
const int PIN_TRIAC = 19;
#define PORT_TRIAC PORTC
#define BIT_TRIAC PC5

/** ZigBee baud rate */
const long XBEE_BAUD_RATE = 57600;

/** LED durations */
const uint8_t LED_BLINK_DURATION = 100;
const uint8_t LED_BLINK_DELAY = 50;

/** When to turn of the LED */
unsigned long ledOffMillis = 0;

/** Next time on which the LED can blink again */
unsigned long ledNextMillis = 0;

/** Key debouncer */
Bounce key(PIN_KEY, 10);

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

/** XBee device */
XXBee<8> xbee;

/** ZigBee reusable RX response message */
ZBRxResponse rxResponse;

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

/** Device state listeners */
ListenerManager<4> listenerManager(& xbee);

/**
 * Notify all listeners about a dimmer value change.
 */
void notifyListeners()
{
	uint8_t message[] = { COYOHO_PWM_READ | COYOHO_MESSAGE_NOTIFY, 0, brightness };
	listenerManager.notifyListeners(message, sizeof(message));
}

/**
 * Flash the LED.
 */
void blinkLED()
{
	if (millis() > ledNextMillis)
	{
		ledOffMillis = millis() + LED_BLINK_DURATION;
		ledNextMillis = ledOffMillis + LED_BLINK_DELAY;
		digitalWrite(PIN_LED, HIGH);
	}
}

/**
 * Switch the LED off if the blink duration elapsed.
 */
void calmDownLED()
{
	if (millis() > ledOffMillis)
	{
		digitalWrite(PIN_LED, LOW);
	}
}

/**
 * Enable timer 1, running with 16/8 = 2MHz.
 * Enable output compare A interrupt.
 */
void configureTimer1()
{
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
 */ISR (TIMER1_COMPA_vect)
{
	if (triggerTriac)
	{
		OCR1A += TRIAC_TRIGGER_DURATION;
		PORT_TRIAC |= (1 << BIT_TRIAC);
		triggerTriac = false;
	}
	else
	{
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
void configureINT0()
{
	pinMode(PIN_INT0, INPUT);
	digitalWrite(PIN_INT0, HIGH);
	EICRA &= ~(1 << ISC00);
	EICRA |= (1 << ISC01);
}

/**
 * Called on each zero crossing of the 50 Hz AC power line.
 */ISR (INT0_vect)
{
	OCR1A = TCNT1 + triacDelay;
	triggerTriac = true;
	PORT_TRIAC &= ~(1 << BIT_TRIAC);
}

/**
 * Set the brightness.
 *
 * @param val The new brightnes value.
 */
void setBrightness(uint8_t val)
{
	blinkLED();
	if (val == 0)
	{
		triacDelay = TRIAC_DELAY_MAX;
		EIMSK &= ~(1 << INT0);
	}
	else
	{
		triacDelay = map(val, 0, 255, TRIAC_DELAY_MAX, TRIAC_DELAY_MIN);
		EIMSK |= (1 << INT0);
	}
	brightness = val;
	if (val == 255)
	{
		brightnessFadeDelta = -1;
	}
	else if (val == 0)
	{
		brightnessFadeDelta = 1;
	}
	notifyListeners();
}

/**
 * System setup.
 */
void setup()
{
	pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_TRIAC, OUTPUT);
	pinMode(PIN_KEY, INPUT);
	digitalWrite(PIN_KEY, HIGH);
	configureINT0();
	configureTimer1();
	Serial.begin(XBEE_BAUD_RATE);
	xbee.begin(Serial);
	wdt_enable(WDTO_2S);
}

/**
 * Main execution loop.
 */
void loop()
{
	wdt_reset();
	calmDownLED();
	listenerManager.checkListenerLeases();

	key.update();
	if (key.fallingEdge())
	{
		keyClickedMillis = millis() + KEY_CLICKED_INTERVAL;
	}
	else if (key.risingEdge() && millis() < keyClickedMillis)
	{
		if (brightnessFadeDelta == 1)
		{
			setBrightness(255);
		}
		else
		{
			setBrightness(0);
		}
	}
	else if (key.read() == LOW && key.duration() > KEY_HOLD_INTERVAL)
	{
		if (millis() > nextBrightnessFadeMillis)
		{
			setBrightness(brightness + brightnessFadeDelta);
			nextBrightnessFadeMillis = millis() + brightnesFadeDelay;
		}
	}

	xbee.readPacket();
	if (xbee.getResponse().isAvailable())
	{
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
		{
			blinkLED();
			xbee.getResponse().getZBRxResponse(rxResponse);
			xbee.resetData(rxResponse.getData(), rxResponse.getDataLength());
			while (xbee.dataAvailable())
			{
				uint8_t command = xbee.getData();

				if (listenerManager.processXBeeMessage(command, xbee, rxResponse))
				{
					continue;
				}

				switch (command)
				{
					case COYOHO_RESET:
						for (;;)
						{
						}
						break;

					case COYOHO_PWM_WRITE:
						if (xbee.dataAvailable(2))
						{
							uint8_t pwmNum = xbee.getData();
							uint8_t mode = xbee.getData();
							if (pwmNum != 0)
							{
								break;
							}
							switch (mode)
							{
								case COYOHO_WRITE_ABSOLUTE:
									if (xbee.dataAvailable())
									{
										setBrightness(xbee.getData());
									}
									break;
								case COYOHO_WRITE_INCREMENT:
									if (xbee.dataAvailable())
									{
										setBrightness(brightness + xbee.getData());
									}
									break;
								case COYOHO_WRITE_INCREMENT_DEFAULT:
									setBrightness(brightness + 10);
									break;
								case COYOHO_WRITE_DECREMENT:
									if (xbee.dataAvailable())
									{
										setBrightness(brightness - xbee.getData());
									}
									break;
								case COYOHO_WRITE_DECREMENT_DEFAULT:
									setBrightness(brightness - 10);
									break;
								case COYOHO_WRITE_TOGGLE:
									setBrightness(brightness != 255 ? 255 : 0);
									break;
								case COYOHO_WRITE_DEFAULT:
								default:
									setBrightness(0);
									break;
							}
						}
						break;

					case COYOHO_PWM_READ:
						if (xbee.dataAvailable(1))
						{
							uint8_t pwmNum = xbee.getData();
							if (pwmNum != 0)
							{
								break;
							}
							xbee.resetPayload();
							xbee.putPayload(COYOHO_PWM_READ | COYOHO_MESSAGE_RESPONSE);
							xbee.putPayload(0);
							xbee.putPayload(brightness);
							ZBTxRequest txRequest(rxResponse.getRemoteAddress64(), xbee.payload(),
									xbee.payloadLenght());
							xbee.send(txRequest);
						}
						break;

					case COYOHO_DUMP:
						xbee.resetPayload();
						xbee.putPayload(COYOHO_DUMP | COYOHO_MESSAGE_RESPONSE);
						xbee.putPayload(COYOHO_DUMP_VERSION);
						xbee.putPayload(COYOHO_VERSION);
						break;
				}
			}
		}
	}
}
