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

/** Digital I/O pin numbers */
const uint8_t PIN_SWITCH1 = 8;
const uint8_t PIN_SWITCH2 = 6;
const uint8_t PIN_SWITCH3 = 4;
const uint8_t PIN_SWITCH4 = 2;
const uint8_t PIN_SWITCH5 = 9;
const uint8_t PIN_SWITCH6 = 7;
const uint8_t PIN_SWITCH7 = 5;
const uint8_t PIN_SWITCH8 = 3;

/** Analog pin numbers */

const uint8_t PIN_CURRENT_SENSE = 0;

/** Switch pins by number */
const uint8_t switchPins[] =
{ PIN_SWITCH1, PIN_SWITCH2, PIN_SWITCH3, PIN_SWITCH4, PIN_SWITCH5, PIN_SWITCH6, PIN_SWITCH7, PIN_SWITCH8 };
const uint8_t NUM_SWITCH_PINS = sizeof switchPins / sizeof switchPins[0];

/** ZigBee baud rate */
const long XBEE_BAUD_RATE = 57600;

/** XBee device */
XXBee<8> xbee;

/** ZigBee reusable RX response message */
ZBRxResponse rxResponse;

/** Current power consumption in W */
uint16_t powerConsumption = 0;

/** Next time to update the sensor values */
unsigned long nextSenseUpdateMillis = 0;

/** Current sensor value maximum */
uint16_t currentSenseMax = 0;

/** Device state listeners */
ListenerManager<4> listenerManager(& xbee);

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
}

void notifyListeners(uint8_t changedSwitchNum)
{
	uint8_t switchPin = switchPins[changedSwitchNum];
	uint8_t message[] = { COYOHO_SWITCH_READ | COYOHO_MESSAGE_NOTIFY, changedSwitchNum,
			digitalRead(switchPin) == HIGH ? 1 : 0 };
	listenerManager.notifyListeners(message, sizeof(message));
}

/**
 * Read the sensor values.
 */
void readSensors()
{
	unsigned long nowMillis = millis();
	if (nowMillis > nextSenseUpdateMillis)
	{
		nextSenseUpdateMillis += 1000;
		powerConsumption = currentSenseMax;
		currentSenseMax = 0;
	}

	uint16_t currentSense = analogRead(PIN_CURRENT_SENSE);
	currentSense = currentSense > 512 ? currentSense - 512 : 512 - currentSense;
	if (currentSense > currentSenseMax)
	{
		currentSenseMax = currentSense;
	}
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
				switch (command)
				{
					case COYOHO_ADD_LISTENER:
						listenerManager.addListener(rxResponse.getRemoteAddress64());
						break;

					case COYOHO_REMOVE_LISTENER:
						listenerManager.removeListener(rxResponse.getRemoteAddress64());
						break;

					case COYOHO_RESET:
						for (uint8_t i = 0; i <= 7; ++i)
						{
							digitalWrite(switchPins[i], LOW);
							notifyListeners(i);
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

					case COYOHO_SENSOR_READ:
						if (xbee.dataAvailable(1))
						{
							if (xbee.getData() == COYOHO_SENSOR_POWER_CONSUMPTION)
							{
								xbee.resetPayload();
								xbee.putPayload(COYOHO_SENSOR_READ | COYOHO_MESSAGE_RESPONSE);
								xbee.putPayload(COYOHO_SENSOR_POWER_CONSUMPTION);
								xbee.putPayload(powerConsumption >> 8);
								xbee.putPayload(powerConsumption & 0xff);
								ZBTxRequest txRequest(rxResponse.getRemoteAddress64(), xbee.payload(),
										xbee.payloadLenght());
								xbee.send(txRequest);
							}
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
 * Main execution loop.
 */
void loop()
{
	listenerManager.checkListenerLeases();
	readSensors();
	processXBeeMessages();
}
