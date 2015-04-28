/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Wire.h>
#include <SoftwareSerial.h>
#include <EEPROMex.h>
#include <WiFly.h>
#include <CmdMessenger.h>
#include <device.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RunningAverage.h>

const uint8_t INFO_LED_PIN = 13;
const uint8_t SYS_BUTTON_PIN = 12;
const uint8_t ONEWIRE_PIN = 8;
const uint8_t PHOTORESISTOR_PIN = A0;

DeviceDescriptor device;

OneWire oneWire(ONEWIRE_PIN);
DallasTemperature temperatureSensor(&oneWire);
float temperature;

RunningAverage brightness(10);

const unsigned long SEND_INTERVAL = 1000;
unsigned long nextSendMillis = 0;

void setup() {
  device.type = "Sensor";
  device.description = "Temperature and brightness sensors demo shield";
  device.ledPin = INFO_LED_PIN;
  device.buttonPin = SYS_BUTTON_PIN;
  device.registerMessageHandlers = register_message_handlers;
  device.sendServerRegisterParams = send_server_register_params;
  deviceInit(device);
}

void loop() {
  deviceUpdate();
  if (deviceIsOperational()) {
    if (millis() > nextSendMillis) {
      digitalWrite(INFO_LED_PIN, digitalRead(INFO_LED_PIN));
      temperatureSensor.requestTemperatures();
      temperature = temperatureSensor.getTempCByIndex(0);
      brightness.addValue((analogRead(PHOTORESISTOR_PIN) * 100.0) / 1024);
      digitalWrite(INFO_LED_PIN, HIGH);
      delay(25);
      digitalWrite(INFO_LED_PIN, LOW);
      switch_read();
      nextSendMillis = millis() + SEND_INTERVAL;
    }
  }
}

void send_server_register_params() {
  device.messenger->sendCmdArg(2);
  device.messenger->sendCmdArg(SENSOR_TEMPERATURE);
  device.messenger->sendCmdArg(-10);
  device.messenger->sendCmdArg(85);
  device.messenger->sendCmdArg(SENSOR_BRIGHTNESS);
  device.messenger->sendCmdArg(0);
  device.messenger->sendCmdArg(100);
}

void register_message_handlers() {
  device.messenger->attach(MSG_SENSOR_READ, switch_read);
}

void switch_read() {
  device.messenger->sendCmdStart(MSG_SENSOR_STATE);
  device.messenger->sendCmdArg(0);
  device.messenger->sendCmdArg(temperature);
  device.messenger->sendCmdArg(1);
  device.messenger->sendCmdArg(brightness.getAverage());
  device.messenger->sendCmdEnd();
}

