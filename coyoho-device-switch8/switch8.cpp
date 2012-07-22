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

#include <Arduino.h>
#include <XXBee/XXBee.h>
#include <CoYoHoMessages.h>
#include <CoYoHoListenerManager.h>
#include <avr/wdt.h>

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

/** ZigBee baud rate */
const long XBEE_BAUD_RATE = 57600;

/** XBee device */
XXBee<8> xbee;

/** ZigBee reusable RX response message */
ZBRxResponse rxResponse;

/** Device state listeners */
ListenerManager<4> listenerManager(&xbee);

/** Beep duration in milli seconds */
const int BEEP_DURATION_SHORT = 100;

/** Beep duration in milli seconds */
const int BEEP_DURATION_LONG = 500;

/** When to switch off the buzzer (milli second timestamp) */
unsigned long beepCalmDownTime = 0;

/**
 * Beep the buzzer.
 *
 * @param duration Beep duration in milli seconds
 */
void beep(int duration = BEEP_DURATION_SHORT)
{
	digitalWrite(PIN_BUZZER, HIGH);
	beepCalmDownTime = millis() + duration;
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
 * Notify all listeners about a state change of the specified switch.
 *
 * @param changedSwitchNum The number of the changed switch
 */
void notifyListeners(uint8_t changedSwitchNum)
{
	uint8_t switchPin = switchPins[changedSwitchNum];
	uint8_t message[] = { COYOHO_SWITCH_READ | COYOHO_MESSAGE_NOTIFY, changedSwitchNum,
			digitalRead(switchPin) == HIGH ? 1 : 0 };
	listenerManager.notifyListeners(message, sizeof(message));
}

/**
 * Process XBee messages.
 */
void processXBeeMessages()
{
	xbee.readPacket();
	if (xbee.getResponse().isAvailable())
	{
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
		{
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

					case COYOHO_SWITCH_WRITE:
						if (xbee.dataAvailable(2))
						{
							uint8_t switchNum = xbee.getData();
							uint8_t switchPin = switchPins[switchNum];
							uint8_t mode = xbee.getData();
							if (0 <= switchNum && switchNum <= 7)
							{
								switch (mode)
								{
									case COYOHO_WRITE_ABSOLUTE:
										if (xbee.dataAvailable())
										{
											digitalWrite(switchPin, xbee.getData() != 0 ? HIGH : LOW);
										}
										break;
									case COYOHO_WRITE_INCREMENT:
										if (xbee.dataAvailable())
										{
											xbee.getData();
											digitalWrite(switchPin, HIGH);
										}
										break;
									case COYOHO_WRITE_INCREMENT_DEFAULT:
										digitalWrite(switchPin, HIGH);
										break;
									case COYOHO_WRITE_DECREMENT:
										if (xbee.dataAvailable())
										{
											xbee.getData();
											digitalWrite(switchPin, LOW);
										}
										break;
									case COYOHO_WRITE_DECREMENT_DEFAULT:
										digitalWrite(switchPin, LOW);
										break;
									case COYOHO_WRITE_TOGGLE:
										digitalWrite(switchPin, digitalRead(switchPin) == LOW ? HIGH : LOW);
										break;
									case COYOHO_WRITE_DEFAULT:
									default:
										digitalWrite(switchPin, LOW);
										break;
								}
								notifyListeners(switchNum);
								beep();
							}
						}
						break;

					case COYOHO_SWITCH_READ:
						if (xbee.dataAvailable(1))
						{
							uint8_t switchNum = xbee.getData();
							xbee.resetPayload();
							xbee.putPayload(COYOHO_SWITCH_READ | COYOHO_MESSAGE_RESPONSE);
							xbee.putPayload(switchNum);
							xbee.putPayload(digitalRead(switchPins[switchNum]) == HIGH ? 1 : 0);
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

/**
 * System setup.
 */
void setup()
{
	for (uint8_t i = 0; i < 8; ++i)
	{
		pinMode(switchPins[i], OUTPUT);
	}
	xbee.begin(XBEE_BAUD_RATE);
	pinMode(PIN_BUZZER, OUTPUT);
	beep(BEEP_DURATION_LONG);
	wdt_enable(WDTO_2S);
}

/**
 * Main execution loop.
 */
void loop()
{
	wdt_reset();
	calmDownBeep();
	listenerManager.checkListenerLeases();
	processXBeeMessages();
}
