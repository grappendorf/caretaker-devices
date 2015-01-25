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
#include <Servo/Servo.h>
#include <DallasTemperature/DallasTemperature.h>
#include <CoYoHoMessages.h>
#include <CoYoHoListenerManager.h>
#include <avr/wdt.h>

/** Digital I/O pin numbers */
const int PIN_STATUS_LED = 13;
const int PIN_AZIMUTH_SERVO = 9;
const int PIN_ALTITUDE_SERVO = 10;
const int PIN_ONE_WIRE = 3;

/** Analog I/O pin numbers */
const int PIN_BRIGHTNESS = 0;

/** Status led durations */
const uint8_t STATUS_LED_BLINK_DURATION = 100;
const uint8_t STATUS_LED_BLINK_DELAY = 50;

/** Temperature sensor request interval in milli seconds */
const uint16_t TEMPERATURE_REQUEST_INTERVAL = 1000;

/** ZigBee baud rate */
const long XBEE_BAUD_RATE = 57600;

/** Azimuth servo min, max, default values */
const int azimuthDefault = 90;
const int azimuthDelta = 5;
const int azimuthMin = 20;
const int azimuthMax = 160;
const int azimuthRangeReductionFactor = 3;

/** Altitude servo min, max, default values */
const int altitudeDefault = 90;
const int altitudeDelta = 2;
const int altitudeMin = 70;
const int altitudeMax = 110;

/** When to turn of the staus LED */
unsigned long statusLedOffMillis = 0;

/** Next time on which the status LED can blink again */
unsigned long statusLedNextMillis = 0;

/** Azimuth servo */
Servo azimuthServo;

/** Altitude servo and min, max, default values */
Servo altitudeServo;

/** One wire device */
OneWire oneWire(PIN_ONE_WIRE);

/** Temperatue sensor */
DallasTemperature temperature(&oneWire);

/** Next temperature request time in milli seconds */
unsigned long nextTeperatureRequestMillis = 0;

/** XBee device */
XXBee<32> xbee;

/** Reusable RX response message */
ZBRxResponse rxResponse;

/** Device state listeners */
ListenerManager<4> listenerManager(&xbee);

/**
 * Flash the status light.
 */
void blinkStatusLight()
{
	if (millis() > statusLedNextMillis)
	{
		statusLedOffMillis = millis() + STATUS_LED_BLINK_DURATION;
		statusLedNextMillis = statusLedOffMillis + STATUS_LED_BLINK_DELAY;
		digitalWrite(PIN_STATUS_LED, HIGH);
	}
}

/**
 * Switch the status LED off if the blink duration elapsed.
 */
void calmDownStatusLight()
{
	if (millis() > statusLedOffMillis)
	{
		digitalWrite(PIN_STATUS_LED, LOW);
	}
}

/**
 * Set a new servo value.
 *
 * @param servo Pointer to the servo to set
 * @param mode Position set mode
 * @param val New sensor value
 * @param min Minimum servo value
 * @param max Maximum servo value
 * @param defaultVal Default servo value
 * @param defaultDelta Default increment/decrement value
 */
void setServo(Servo *servo, int mode, int val, int min, int max, int defaultVal, int defaultDelta)
{
	int pos = servo->read();
	switch (mode)
	{
		case COYOHO_WRITE_ABSOLUTE:
			pos = val;
			break;
		case COYOHO_WRITE_DEFAULT:
			pos = defaultVal;
			break;
		case COYOHO_WRITE_INCREMENT:
			pos += val;
			break;
		case COYOHO_WRITE_INCREMENT_DEFAULT:
			pos += defaultDelta;
			break;
		case COYOHO_WRITE_DECREMENT:
			pos -= val;
			break;
		case COYOHO_WRITE_DECREMENT_DEFAULT:
			pos -= defaultDelta;
			break;
	}
	servo->write(constrain (pos, min, max));
}

/**
 * Perform a servo write command.
 */
void cmdServoWrite()
{
	while (xbee.dataAvailable(2))
	{
		int servo = xbee.getData();
		int mode = xbee.getData();
		int val = 0;

		if ((mode == COYOHO_WRITE_ABSOLUTE || mode == COYOHO_WRITE_INCREMENT || mode == COYOHO_WRITE_DECREMENT)
				&& xbee.dataAvailable())
		{
			val = xbee.getData();
		}

		if (servo == COYOHO_SERVO_ALTITUDE || servo == COYOHO_SERVO_ALL)
		{
			setServo(&altitudeServo, mode, val, altitudeMin, altitudeMax, altitudeDefault, altitudeDelta);
		}

		int rangeReduction = abs(altitudeServo.read() - altitudeDefault) * azimuthRangeReductionFactor;
		int min = azimuthMin + rangeReduction;
		int max = azimuthMax - rangeReduction;
		azimuthServo.write(constrain (azimuthServo.read(), min, max));

		if (servo == COYOHO_SERVO_AZIMUTH || servo == COYOHO_SERVO_ALL)
		{
			setServo(&azimuthServo, mode, val, min, max, azimuthDefault, azimuthDelta);
		}
	}
}

/**
 * Perform a servo read command.
 */
void cmdServoRead()
{
	xbee.resetPayload();
	xbee.putPayload(COYOHO_SERVO_READ | COYOHO_MESSAGE_RESPONSE);
	bool azimuthServoWritten = false;
	bool altitudeServoWritten = false;
	while (xbee.dataAvailable())
	{
		uint8_t servoNum = xbee.getData();
		if ((servoNum == COYOHO_SERVO_AZIMUTH || servoNum == COYOHO_SERVO_ALL) && !azimuthServoWritten)
		{
			xbee.putPayload(COYOHO_SERVO_AZIMUTH);
			xbee.putPayload(azimuthServo.read());
			azimuthServoWritten = true;
		}
		else if ((servoNum == COYOHO_SERVO_ALTITUDE || servoNum == COYOHO_SERVO_ALL) && !altitudeServoWritten)
		{
			xbee.putPayload(COYOHO_SERVO_ALTITUDE);
			xbee.putPayload(altitudeServo.read());
			altitudeServoWritten = true;
		}
	}
	ZBTxRequest txRequest(rxResponse.getRemoteAddress64(), xbee.payload(), xbee.payloadLenght());
	xbee.send(txRequest);
}

/**
 * Perform a sensor read command.
 */
void cmdSensorRead()
{
	xbee.resetPayload();
	xbee.putPayload(COYOHO_SENSOR_READ | COYOHO_MESSAGE_RESPONSE);
	bool temperatureWritten = false;
	bool brightnessWritten = false;
	bool azimuthServoWritten = false;
	bool altitudeServoWritten = false;
	while (xbee.dataAvailable())
	{
		uint8_t sensorType = xbee.getData();
		uint8_t sensorNum = COYOHO_SENSOR_ALL;
		if (sensorType != COYOHO_SENSOR_ALL)
		{
			if (!xbee.dataAvailable())
			{
				continue;
			}
			sensorNum = xbee.getData();
		}
		if ((sensorType == COYOHO_SENSOR_TEMPERATURE || sensorType == COYOHO_SENSOR_ALL) && !temperatureWritten)
		{
			uint16_t val = temperature.getTempCByIndex(0) * 100;
			xbee.putPayload(COYOHO_SENSOR_TEMPERATURE);
			xbee.putPayload(0);
			xbee.putPayload((val >> 8) & 0xff);
			xbee.putPayload(val & 0xff);
			temperatureWritten = true;
		}
		if ((sensorType == COYOHO_SENSOR_BRIGHTNESS || sensorType == COYOHO_SENSOR_ALL) && !brightnessWritten)
		{
			uint16_t val = 1023 - analogRead(PIN_BRIGHTNESS);
			xbee.putPayload(COYOHO_SENSOR_BRIGHTNESS);
			xbee.putPayload(0);
			xbee.putPayload((val >> 8) & 0xff);
			xbee.putPayload(val & 0xff);
			brightnessWritten = true;
		}
		if ((sensorType == COYOHO_SENSOR_SERVO || sensorType == COYOHO_SENSOR_ALL)
				&& (sensorNum == COYOHO_SERVO_AZIMUTH || sensorNum == COYOHO_SERVO_ALL) && !azimuthServoWritten)
		{
			xbee.putPayload(COYOHO_SENSOR_SERVO);
			xbee.putPayload(COYOHO_SERVO_AZIMUTH);
			xbee.putPayload(180 - azimuthServo.read());
			azimuthServoWritten = true;
		}
		if ((sensorType == COYOHO_SENSOR_SERVO || sensorType == COYOHO_SENSOR_ALL)
				&& (sensorNum == COYOHO_SERVO_ALTITUDE || sensorNum == COYOHO_SERVO_ALL) && !altitudeServoWritten)
		{
			xbee.putPayload(COYOHO_SENSOR_SERVO);
			xbee.putPayload(COYOHO_SERVO_ALTITUDE);
			xbee.putPayload(altitudeServo.read());
			altitudeServoWritten = true;
		}
	}
	ZBTxRequest txRequest(rxResponse.getRemoteAddress64(), xbee.payload(), xbee.payloadLenght());
	xbee.send(txRequest);
}

/**
 * System setup.
 */
void setup()
{
	pinMode(PIN_STATUS_LED, OUTPUT);
	azimuthServo.attach(PIN_AZIMUTH_SERVO);
	altitudeServo.attach(PIN_ALTITUDE_SERVO);
	azimuthServo.write(azimuthDefault);
	altitudeServo.write(altitudeDefault);
	temperature.begin();
	Serial.begin(XBEE_BAUD_RATE);
	xbee.begin(Serial);
	blinkStatusLight();
	wdt_enable(WDTO_2S);
}

/**
 * Main execution loop.
 */
void loop()
{
	wdt_reset();
	calmDownStatusLight();
	listenerManager.checkListenerLeases();

	xbee.readPacket();
	if (xbee.getResponse().isAvailable())
	{
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
		{
			xbee.getResponse().getZBRxResponse(rxResponse);
			xbee.resetData(rxResponse.getData(), rxResponse.getDataLength());
			if (xbee.dataAvailable())
			{
				uint8_t command = xbee.getData();

				if (listenerManager.processXBeeMessage(command, xbee, rxResponse))
				{
					blinkStatusLight();
				}
				else
				{
					switch (command)
					{
						case COYOHO_RESET:
							for (;;)
							{
							}
							break;

						case COYOHO_SENSOR_READ:
							blinkStatusLight();
							cmdSensorRead();
							break;

						case COYOHO_SERVO_WRITE:
							blinkStatusLight();
							cmdServoWrite();
							break;

						case COYOHO_SERVO_READ:
							blinkStatusLight();
							cmdServoRead();
							break;

						case COYOHO_DUMP:
							blinkStatusLight();
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
	if (millis() > nextTeperatureRequestMillis)
	{
		temperature.requestTemperatures();
		nextTeperatureRequestMillis = millis() + TEMPERATURE_REQUEST_INTERVAL;
	}
}
