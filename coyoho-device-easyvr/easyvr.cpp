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
#include <SoftwareSerial/SoftwareSerial.h>
#include <EasyVR/EasyVR.h>
#include <XXBee/XXBee.h>
#include <CoYoHoMessages.h>
#include <CoYoHoListenerManager.h>
#include <avr/wdt.h>

const int PIN_LED = 10;
const int PIN_XBEE_RXD = 0;
const int PIN_XBEE_TXD = 1;
const int PIN_USB_RXD = 2;
const int PIN_USB_TXD = 3;
const int PIN_EASYVR_RXD = 5;
const int PIN_EASYVR_TXD = 4;

const long USB_BAUD_RATE = 9600;
const long XBEE_BAUD_RATE = 57600;
const long EASYVR_BAUD_RATE = 9600;

XXBee<8> xbee;
ZBRxResponse rxResponse;
ListenerManager<4> listenerManager(&xbee);
//SoftwareSerial usbSerial(PIN_USB_RXD, PIN_USB_TXD);
SoftwareSerial easyvrSerial(PIN_EASYVR_RXD, PIN_EASYVR_TXD);
EasyVR easyvr(easyvrSerial);

int wordGroup = -1;
int wordIndex = -1;

/** States of the device state engine */
enum State
{
	RECOGNIZE_TRIGGER, RECOGNIZE_GROUP, RECOGNIZE_WORD
};

/** Current device mode state */
State state = RECOGNIZE_TRIGGER;

void error(int num)
{
	for (;;)
	{
		for (int i = 0; i < num; ++i)
		{
			digitalWrite(PIN_LED, HIGH);
			delay(250);
			digitalWrite(PIN_LED, LOW);
			delay(250);
		}
		delay(1000);
	}
}

void blinkLed()
{
	digitalWrite(PIN_LED, HIGH);
	delay(50);
	digitalWrite(PIN_LED, LOW);
}

void notifyListeners(uint8_t button)
{
	uint8_t message1[] = { COYOHO_SWITCH_READ | COYOHO_MESSAGE_NOTIFY, button, 1 };
	listenerManager.notifyListeners(message1, sizeof(message1));
	delay(500);
	uint8_t message2[] = { COYOHO_SWITCH_READ | COYOHO_MESSAGE_NOTIFY, button, 0 };
	listenerManager.notifyListeners(message2, sizeof(message2));
}

void setup()
{
	pinMode(PIN_LED, OUTPUT);
	digitalWrite(PIN_LED, HIGH);
	delay(500);

	easyvrSerial.begin(EASYVR_BAUD_RATE);
	if (! easyvr.detect())
	{
		error(1);
	}
	easyvr.setTimeout(0);
	easyvr.setLanguage(EasyVR::ENGLISH);

//	usbSerial.begin(USB_BAUD_RATE);

	Serial.begin(XBEE_BAUD_RATE);
	xbee.begin(Serial);

	digitalWrite(PIN_LED, LOW);
    easyvr.playSound(2, EasyVR::VOL_DOUBLE);
	easyvr.recognizeCommand(0);
	wdt_enable(WDTO_2S);
}

void loop()
{
	wdt_reset();
	listenerManager.checkListenerLeases();

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
						digitalWrite(PIN_LED, HIGH);
						for (;;)
						{
						}
						break;
				}
			}
		}
	}

	switch (state)
	{
		case RECOGNIZE_TRIGGER:
			if (easyvr.hasFinished())
			{
				blinkLed();
				int wordTrigger = easyvr.getCommand();
				if (wordTrigger >= 0)
				{
					easyvr.recognizeCommand(1);
					state = RECOGNIZE_GROUP;
				}
				else
				{
				    easyvr.playSound(2, EasyVR::VOL_DOUBLE);
					easyvr.recognizeCommand(0);
				}
			}
			break;

		case RECOGNIZE_GROUP:
			if (easyvr.hasFinished())
			{
				blinkLed();
				wordGroup = easyvr.getCommand();
				if (wordGroup >= 0)
				{
					easyvr.recognizeCommand(1 + wordGroup);
					state = RECOGNIZE_WORD;
				}
				else
				{
				    easyvr.playSound(2, EasyVR::VOL_DOUBLE);
					easyvr.recognizeCommand(0);
					state = RECOGNIZE_TRIGGER;
				}
			}
			break;

		case RECOGNIZE_WORD:
			if (easyvr.hasFinished())
			{
				blinkLed();
				wordIndex = easyvr.getCommand();
				if (wordIndex >= 0)
				{
			        notifyListeners(wordIndex - 1);
					easyvr.recognizeCommand(0);
					state = RECOGNIZE_TRIGGER;
				}
				else
				{
				    easyvr.playSound(2, EasyVR::VOL_DOUBLE);
					easyvr.recognizeCommand(0);
					state = RECOGNIZE_TRIGGER;
				}
			}
			break;
	}
}
