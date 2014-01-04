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
#include <XXBee/XXBee.h>
#include <CoYoHoMessages.h>
#include <CoYoHoListenerManager.h>

/** IO Pins */
const uint8_t RXD = 0;
const uint8_t TXD = 1;
const uint8_t XBEE_SLEEP = 2;
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

/** XBee device */
XXBee<8> xbee;
ZBTxStatusResponse txStatusResponse;
ZBRxResponse rxResponse;
const long XBEE_BAUD_RATE = 57600;

/** Listeners */
ListenerManager<4> listenerManager(& xbee);

/** Some LED constants */
enum Led
{
	NONE = 0, YELLOW = 1, GREEN = 2, RED = 4
};

/** Button pin numbers */
uint8_t buttons[] = { BUTTON_01, BUTTON_02, BUTTON_03, BUTTON_04, BUTTON_05, BUTTON_06, BUTTON_07, BUTTON_08, BUTTON_09,
		BUTTON_10 };

/** Number of buttons */
const uint8_t NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

uint8_t lastButtonStates[NUM_BUTTONS];

/** Convenient aliases */
const uint8_t BUTTON_PRESSED = LOW;
const uint8_t BUTTON_RELEASED = HIGH;

/** Indicates that no command should be defined for a button */
const uint8_t UNDEFINED = 0xff;

/** Indicates that listeners should be notified when a button is pressed or released */
const uint8_t LISTENERS = 0xfe;

/**
 * XBee message codes. First comes the codes that are sent when a button is pressed,
 * followed by the codes sent when a button is released
 */
const uint8_t EEMEM codeData[] = {

		// Page 0: Only notify the listeners

		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,
		LISTENERS,

		// Page 1: Fail back, direct control mode

		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 0, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 1, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x97, 0x3d, 0x48, 0x40, 3, COYOHO_PWM_WRITE, 0, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 2, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x9d, 0x31, 0x61, 0x40, 3, COYOHO_RGB_WRITE, 0, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 4, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 5, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 6, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 6, COYOHO_WRITE_TOGGLE,
		UNDEFINED,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 6, COYOHO_WRITE_TOGGLE,
		UNDEFINED,

		// Page 2: Experimental mode

		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 4, COYOHO_SWITCH_WRITE, 0, COYOHO_WRITE_ABSOLUTE, 1,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 4, COYOHO_SWITCH_WRITE, 0, COYOHO_WRITE_ABSOLUTE, 0,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
		UNDEFINED,
};

/** Switch beetween various pages of codes */
uint8_t codeIndexOffset = 0;

const uint8_t NUM_CODE_PAGES = 3;

/**
 * Pointers into the code tables. Code index for button press at even indices, code index
 * for button release at odd indices
 */
uint16_t codeIndex[NUM_CODE_PAGES * NUM_BUTTONS * 2];

/** Indicates that no button has changed its state since the last check */
const uint8_t NO_KEY_CHANGED = 0xff;

/** States of the device mode state engine */
enum State
{
	POWER_UP, NORMAL_MODE, PROGRAMMING_MODE
};

/** Current device mode state */
State state = POWER_UP;

/**
 * This function creates a table of pointers into the button code table.
 * codeIndex then contains the code position for every page, ever button
 * and each of the pressed and released states.
 */
void updateCodeIndices()
{
	uint16_t p = 0;
	for (uint8_t i = 0; i < NUM_CODE_PAGES * NUM_BUTTONS * 2; ++i)
	{
		uint8_t buttonType =eeprom_read_byte(codeData + p);
		if (buttonType == LISTENERS || buttonType == UNDEFINED)
		{
			codeIndex[i] = 0xff00 + buttonType;
			++p;
		}
		else
		{
			uint8_t length = eeprom_read_byte(codeData + p + 8);
			codeIndex[i] = p;
			p += 8 + 1 + length;
		}
	}
}

/**
 * Enable all specified leds.
 */
void lightLeds(uint8_t leds)
{
	digitalWrite(LED_YELLOW, leds & YELLOW ? HIGH : LOW);
	digitalWrite(LED_GREEN, leds & GREEN ? HIGH : LOW);
	digitalWrite(LED_RED, leds & RED ? HIGH : LOW);
}

/**
 * Flash all specified leds.
 */
void flashLeds(uint8_t leds, uint16_t interval = 50)
{
	lightLeds(leds);
	delay(interval);
	lightLeds(NONE);
}

/**
 * Flash all specified leds.
 */
void blinkLeds(uint8_t leds, uint16_t intervalOn = 50, uint16_t intervalOff = 50)
{
	flashLeds(leds, intervalOn);
	delay(intervalOff);
}

/**
 * Nothing done here. Pin change interrupt simply quits sleep mode
 */
void buttonPressed()
{
}

void xbeeSleep(bool sleep)
{
	digitalWrite(XBEE_SLEEP, sleep ? HIGH : LOW);
}

/**
 * Wait until an ongoing sending serial communication with
 * the XBee module is finished.
 */
void waitForXBeeSendFinished()
{
	while ((UCSR0A & (1 << TXC0)) == 0)
		;
}

/**
 * Check for low battery state and inform the user.
 */
void checkLowBattery()
{
	if (digitalRead(LOW_BAT) == LOW)
	{
		blinkLeds(RED | GREEN | YELLOW);
	}
}

/**
 * Notify all registered listeners about a button state change.
 */
void notifyListeners(uint8_t button, uint8_t buttonState)
{
	uint8_t message[] = { COYOHO_SWITCH_READ | COYOHO_MESSAGE_NOTIFY, button,
			buttonState == BUTTON_PRESSED ? 1 : 0 };
	listenerManager.notifyListeners(message, sizeof(message));
}

/**
 * Send the XBee message of the specifed button.
 */
void sendMessageOfButton(uint8_t button, uint8_t buttonState)
{
	uint16_t c = codeIndex[codeIndexOffset + button * 2 + (buttonState == BUTTON_PRESSED ? 0 : 1)];
	if (c == 0xff00 + LISTENERS)
	{
		flashLeds(YELLOW);
		notifyListeners(button, buttonState);
		flashLeds(GREEN);
		waitForXBeeSendFinished();
	}
	else if (c != 0xff00 + UNDEFINED)
	{
		flashLeds(YELLOW);
		uint32_t addrHi = eeprom_read_dword((uint32_t *) (codeData + c));
		c += 4;
		uint32_t addrLo = eeprom_read_dword((uint32_t *) (codeData + c));
		c += 4;
		XBeeAddress64 addr(addrHi, addrLo);
		xbee.resetPayload();
		uint8_t len = eeprom_read_byte(codeData + c);
		++c;
		for (uint8_t j = 0; j < len; ++j)
		{
			xbee.putPayload(eeprom_read_byte(codeData + c));
			++c;
		}
		ZBTxRequest txRequest(addr, xbee.payload(), xbee.payloadLenght());
		xbee.send(txRequest);
		boolean ok = false;
		if (xbee.readPacket(2000))
		{
			if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
			{
				xbee.getResponse().getZBTxStatusResponse(txStatusResponse);
				if (txStatusResponse.getDeliveryStatus() == SUCCESS)
				{
					ok = true;
				}
			}
		}
		flashLeds(ok ? GREEN : RED);
		waitForXBeeSendFinished();
	}
}

/**
 * Return the number of the pressed button (0..9) or NO_KEY_CHANEGD if none was pressed.
 */
uint8_t checkButtons()
{
	uint8_t changedKey = NO_KEY_CHANGED;
	for (uint8_t i = 0; i < NUM_BUTTONS; ++i)
	{
		uint8_t buttonState = digitalRead(buttons[i]);
		if (buttonState != lastButtonStates[i])
		{
			lastButtonStates[i] = buttonState;
			changedKey = i;
		}
	}
	return changedKey;
}

/**
 * As long as the state of any button changes, sent the assigned XBee messages.
 * A button change can change during this message sending, so we must loop and only
 * return if there was no more change.
 */
void checkAndReactToButtonChanges()
{
	uint8_t changedButton = checkButtons();
	while (changedButton != NO_KEY_CHANGED)
	{
		sendMessageOfButton(changedButton, lastButtonStates[changedButton]);
		changedButton = checkButtons();
	}
}

/**
 * Check if we got an incoming XBee message and react to it.
 */
void checkAndReactToIncomingXBeeMesage()
{
	xbee.readPacket();
	if (xbee.getResponse().isAvailable())
	{
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
		{
			flashLeds(RED | GREEN | YELLOW);

			xbee.getResponse().getZBRxResponse(rxResponse);
			xbee.resetData(rxResponse.getData(), rxResponse.getDataLength());
			if (xbee.dataAvailable(1))
			{
				uint8_t command = xbee.getData();
				switch (command)
				{
					case COYOHO_PROGRAM_WRITE:
					{
						uint8_t *addr = (uint8_t *) codeData;
						while (xbee.dataAvailable())
						{
							eeprom_write_byte(addr++, xbee.getData());
						}
						updateCodeIndices();
						flashLeds(RED | GREEN | YELLOW);
						flashLeds(RED | GREEN | YELLOW);
						break;
					}

					case COYOHO_SWITCH_WRITE:
						if (xbee.dataAvailable(2))
						{
							uint8_t switchNum = xbee.getData();
							uint8_t mode = xbee.getData();
							if (0 <= switchNum && switchNum <= 9)
							{
								switch (mode)
								{
									default:
										// Every switch message type simulates a button press
										sendMessageOfButton(switchNum, BUTTON_PRESSED);
										sendMessageOfButton(switchNum, BUTTON_RELEASED);
										break;
								}
							}
						}
						break;

					default:
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
	// TODO: Since the remote control device is in deep sleep mode all the time, we cannot listen
	// to incoming listener requests (for this to happen we need to enable wake up on UART).
	// To send listener messages to the server, here we hard coded the listeners.
	listenerManager.addListener(XBeeAddress64(0x0013a200, 0x406131ae));
	listenerManager.addListener(XBeeAddress64(0x0013a200, 0x404a500b));
	updateCodeIndices();
	xbeeSleep(true);
	pinMode(XBEE_SLEEP, OUTPUT);
	lightLeds(NONE);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_YELLOW, OUTPUT);
	for (uint8_t i = 0; i < NUM_BUTTONS; ++i)
	{
		lastButtonStates[i] = BUTTON_RELEASED;
		digitalWrite(buttons[i], HIGH);
		pinMode(buttons[i], INPUT);
		PCattachInterrupt(buttons[i], buttonPressed, RISING);
	}
	xbee.begin(XBEE_BAUD_RATE);
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
void loop()
{
	// TODO: Currently we don't check the listener leases. See the comment about listeners
	// in the setup function.
	//listenerManager.checkListenerLeases();
	checkLowBattery();
	switch (state)
	{
		case POWER_UP:
		{
			uint8_t button = checkButtons();
			if (button != NO_KEY_CHANGED)
			{
				for (uint8_t i = 0; i <= button; ++i)
				{
					blinkLeds(RED | GREEN | YELLOW);
				}
				if (button == 9)
				{
					// Pressing button 10 when powering up, enter programming mode
					xbeeSleep(false);
					state = PROGRAMMING_MODE;
				}
				else
				{
					// Pressing button 1 to 9 when powering up, select another button page
					codeIndexOffset = (button + 1) * NUM_BUTTONS * 2;
					state = NORMAL_MODE;
				}
			}
			else
			{
				state = NORMAL_MODE;
			}
			break;
		}

		case NORMAL_MODE:
			xbeeSleep(true);
			sleep_enable ();
			sleep_mode ();
			sleep_disable ();
			xbeeSleep(false);
			checkAndReactToButtonChanges();
			break;

		case PROGRAMMING_MODE:
			checkAndReactToIncomingXBeeMesage();
			break;
	}
}
