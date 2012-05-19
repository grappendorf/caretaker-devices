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
#include <PinChangeInt/PinChangeInt.h>
#include <XXBee/XXBee.h>
#include <CoYoHoMessages.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/io.h>
#include <avr/eeprom.h>

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

enum Led
{
	NONE = 0, YELLOW = 1, GREEN = 2, RED = 4
};

/** ZigBee baud rate */
const long XBEE_BAUD_RATE = 57600;

/** XBee device */
XXBee<8> xbee;

/** Button pins */
uint8_t buttons[] = { BUTTON_01, BUTTON_02, BUTTON_03, BUTTON_04, BUTTON_05, BUTTON_06, BUTTON_07, BUTTON_08, BUTTON_09,
		BUTTON_10 };

/** Number of buttons */
const uint8_t numButtons = sizeof(buttons) / sizeof(buttons[0]);

/** XBee TX status response */
ZBTxStatusResponse txStatusResponse;

/** ZigBee reusable RX response message */
ZBRxResponse rxResponse;

/** Indicates that no cmommand is defined for a button */
const uint16_t UNDEF = 0xffff;

/** Pointers into the code table */
const uint16_t EEMEM codes[10] = { 0, 12, 24, 36, 48, 60, 72, 84, 99, 114 };

/** XBee message codes */
const uint8_t EEMEM codeData[] = {
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 0, COYOHO_WRITE_TOGGLE,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 1, COYOHO_WRITE_TOGGLE,
		0x00, 0xa2, 0x13, 0x00, 0x97, 0x3d, 0x48, 0x40, 3, COYOHO_PWM_WRITE, 0, COYOHO_WRITE_TOGGLE,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 2, COYOHO_WRITE_TOGGLE,
		0x00, 0xa2, 0x13, 0x00, 0x9d, 0x31, 0x61, 0x40, 3, COYOHO_RGB_WRITE, 0, COYOHO_WRITE_TOGGLE,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 4, COYOHO_WRITE_TOGGLE,
		0x00, 0xa2, 0x13, 0x00, 0x0c, 0x50, 0x4a, 0x40, 3, COYOHO_SWITCH_WRITE, 5, COYOHO_WRITE_TOGGLE,
		0x00, 0xa2, 0x13, 0x00, 0x9d, 0x31, 0x61, 0x40, 6, COYOHO_RGB_WRITE, 0, COYOHO_WRITE_INCREMENT, 32, 0, 0,
		0x00, 0xa2, 0x13, 0x00, 0x9d, 0x31, 0x61, 0x40, 6, COYOHO_RGB_WRITE, 0, COYOHO_WRITE_INCREMENT, 0, 32, 0,
		0x00, 0xa2, 0x13, 0x00, 0x9d, 0x31, 0x61, 0x40, 6, COYOHO_RGB_WRITE, 0, COYOHO_WRITE_INCREMENT, 0, 0, 32,};

enum State
{
	POWER_UP, NORMAL_MODE, PROGRAMMING_MODE
};

State state = POWER_UP;

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
void flashLeds(uint8_t leds, uint16_t interval = 200)
{
	lightLeds(leds);
	delay(interval);
	lightLeds(NONE);
}

/**
 * Flash all specified leds.
 */
void blinkLeds(uint8_t leds, uint16_t intervalOn = 200, uint16_t intervalOff = 200)
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
 * Send the XBee message of the specifed button.
 */
void sendMessageOfButton(uint8_t button)
{
	uint16_t c = eeprom_read_word(codes + button);
	if (c != UNDEF)
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
 * Return the number of the pressed button (0..9) or -1 if none was pressed.
 * @return
 */
int8_t checkButtons()
{
	for (uint8_t i = 0; i < numButtons; ++i)
	{
		if (digitalRead(buttons[i]) == LOW)
		{
			return i;
		}
	}
	return -1;
}

/**
 * Check if any buttons are pressed and react to it, i.e. send the programmed
 * XBee messages.
 */
void checkAndReactToButtons()
{
	int8_t button = checkButtons();
	if (button != -1)
	{
		sendMessageOfButton(button);
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
						if (xbee.dataAvailable(20))
						{
							uint8_t *addr = (uint8_t *) codes;
							for (uint8_t i = 0; i < 20; ++i)
							{
								eeprom_write_byte(addr++, xbee.getData());
							}
							addr = (uint8_t *) codeData;
							while (xbee.dataAvailable())
							{
								eeprom_write_byte(addr++, xbee.getData());
							}
							flashLeds(RED | GREEN | YELLOW);
							flashLeds(RED | GREEN | YELLOW);
						}
						break;

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
										sendMessageOfButton(switchNum);
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
 * System setup.
 */
void setup()
{
	xbeeSleep(true);
	pinMode(XBEE_SLEEP, OUTPUT);
	lightLeds(NONE);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_YELLOW, OUTPUT);
	for (uint8_t i = 0; i < numButtons; ++i)
	{
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
 * Main execution loop.
 */
void loop()
{
	checkLowBattery();
	switch (state)
	{
		case POWER_UP:
			if (checkButtons() != -1)
			{
				blinkLeds(RED | GREEN | YELLOW);
				xbeeSleep(false);
				state = PROGRAMMING_MODE;
			}
			else
			{
				state = NORMAL_MODE;
			}
			break;

		case NORMAL_MODE:
			xbeeSleep(true);
			sleep_enable ();
			sleep_mode ();
			sleep_disable ();
			xbeeSleep(false);
			checkAndReactToButtons();
			break;

		case PROGRAMMING_MODE:
			checkAndReactToIncomingXBeeMesage();
			break;
	}
}
