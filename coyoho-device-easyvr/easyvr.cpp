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
#include <WiFlySerial/WiFlySerial.h>
#include <SoftwareSerial/SoftwareSerial.h>

const int PIN_LED = 10;
const int PIN_WIFLY_RXD = 3;
const int PIN_WIFLY_TXD = 2;

const int USB_BAUD_RATE = 9600;
const int WIFLY_BAUD_RATE = 9600;

WiFlySerial wifly(PIN_WIFLY_RXD, PIN_WIFLY_TXD);

void setup()
{
	pinMode(PIN_LED, OUTPUT);
	Serial.begin(USB_BAUD_RATE);
	wifly.begin();
}

void loop()
{
	digitalWrite(10, HIGH);
	delay(200);
	digitalWrite(10, LOW);
	delay(200);
}
