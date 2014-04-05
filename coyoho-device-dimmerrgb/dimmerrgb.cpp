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
#include <Bounce/Bounce.h>
#include <XXBee/XXBee.h>
#include <CoYoHoMessages.h>
#include <CoYoHoListenerManager.h>
#include <avr/wdt.h>

/** IO pin numbers */
const uint8_t PIN_LED_RED = 5;
const uint8_t PIN_LED_GREEN = 6;
const uint8_t PIN_LED_BLUE = 9;
const uint8_t PIN_BUTTON = 7;

/** XBee module */
XXBee<8> xbee;
ZBRxResponse rxResponse;
const long XBEE_BAUD_RATE = 57600;

/** Listeners */
ListenerManager<4> listenerManager(& xbee);

/** Top switch button */
Bounce button(PIN_BUTTON, 10);

/** Current RGB values */
uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;

/**
 * Notify all registered listeners about the current RGB values.
 */
void notifyListeners()
{
	uint8_t rgbNum = 0;
	uint8_t message[] = { COYOHO_RGB_READ | COYOHO_MESSAGE_NOTIFY, rgbNum,
			red, green, blue };
	listenerManager.notifyListeners(message, sizeof(message));
}

/**
 * Set the RGB values.
 */
void rgb(uint8_t r, uint8_t g, uint8_t b)
{
	red = r;
	green = g;
	blue = b;
	analogWrite(PIN_LED_RED, red);
	analogWrite(PIN_LED_GREEN, green);
	analogWrite(PIN_LED_BLUE, blue);
	notifyListeners();
}

/**
 * Toggle between on (white) and off.
 */
void toggle()
{
	if (red > 0 || green > 0 || blue > 0)
	{
		rgb(0, 0, 0);
	}
	else
	{
		rgb(255, 255, 255);
	}
}

/**
 * Process all received XBee messages.
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

					case COYOHO_RGB_WRITE:
						if (xbee.dataAvailable(2))
						{
							uint8_t rgbNum = xbee.getData();
							uint8_t mode = xbee.getData();
							if (rgbNum != 0)
							{
								break;
							}
							switch (mode)
							{
								case COYOHO_WRITE_ABSOLUTE:
									if (xbee.dataAvailable(3))
									{
										rgb(xbee.getData(), xbee.getData(), xbee.getData());
									}
									break;
								case COYOHO_WRITE_INCREMENT:
									if (xbee.dataAvailable(3))
									{
										rgb(red + xbee.getData(), green + xbee.getData(), blue + xbee.getData());
									}
									break;
								case COYOHO_WRITE_INCREMENT_DEFAULT:
									rgb(red + 32, green + 32, blue + 32);
									break;
								case COYOHO_WRITE_DECREMENT:
									if (xbee.dataAvailable(3))
									{
										rgb(red - xbee.getData(), green - xbee.getData(), blue - xbee.getData());
									}
									break;
								case COYOHO_WRITE_DECREMENT_DEFAULT:
									rgb(red + 32, green + 32, blue + 32);
									break;
								case COYOHO_WRITE_TOGGLE:
									toggle();
									break;
								case COYOHO_WRITE_DEFAULT:
								default:
									rgb(0, 0, 0);
									break;
							}
						}
						break;

					case COYOHO_RGB_READ:
						if (xbee.dataAvailable(1))
						{
							uint8_t rgbNum = xbee.getData();
							if (rgbNum != 0)
							{
								break;
							}
							xbee.resetPayload();
							xbee.putPayload(COYOHO_RGB_READ | COYOHO_MESSAGE_RESPONSE);
							xbee.putPayload(rgbNum);
							xbee.putPayload(red);
							xbee.putPayload(green);
							xbee.putPayload(blue);
							ZBTxRequest txRequest(rxResponse.getRemoteAddress64(), xbee.payload(),
									xbee.payloadLenght());
							txRequest.setAddress16(rxResponse.getRemoteAddress16());
							xbee.send(txRequest);
						}
						break;

					case COYOHO_DUMP:
						xbee.resetPayload();
						xbee.putPayload(COYOHO_DUMP | COYOHO_MESSAGE_RESPONSE);
						xbee.putPayload(COYOHO_DUMP_VERSION);
						xbee.putPayload(COYOHO_VERSION);
						ZBTxRequest txRequest(rxResponse.getRemoteAddress64(), xbee.payload(),
								xbee.payloadLenght());
						txRequest.setAddress16(rxResponse.getRemoteAddress16());
						xbee.send(txRequest);
						break;
				}
			}
		}
	}
}

/**
 * Main system setup.
 */
void setup()
{
	pinMode(PIN_LED_RED, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);
	pinMode(PIN_LED_BLUE, OUTPUT);
	pinMode(PIN_BUTTON, INPUT);
	digitalWrite(PIN_BUTTON, HIGH);
	xbee.begin(XBEE_BAUD_RATE);
	wdt_enable(WDTO_2S);
}

/**
 * Main system loop.
 */
void loop()
{
	wdt_reset();
	listenerManager.checkListenerLeases();
	processXBeeMessages();
	button.update();
	if (button.fallingEdge())
	{
		toggle();
	}
}
