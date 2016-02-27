Caretaker Smart Home Server Devices
===================================

Caretaker is a DIY smart home project. The aim is to create a totally wireless and scaleable system to control 
power switches (lamps and other electrical appliances) and measure values like temperatures, brightness and power 
consumption. All devices should be controllable through local devices as well as mobile devices over the internet. 
The goal is to use WiFly WLAN modules in every device. This allows for a bi-directional communication between all 
devices, so besides controlling a device it can itself send asynchronous status update messages, i.e. pressing a 
physical button on a dimmer to switch a lamp on, will also update a button in a remote control application, 
eliminating the need for periodically polling the device status

This project contains the firmware and Eagle PCB schematics and layouts for the various Caretaker devices.


Licenses
--------

The firmare source code is licensed under the MIT license.
The hardware is licensed under the CERN Open Hardware Licence v1.2.


Third Party Libraries
----------------------

The following Arduino libraries are included into this project:

* Adafruit_MAX31855
	https://github.com/adafruit/Adafruit-MAX31855-library
	BSD
* Bounce 
  http://www.arduino.cc/playground/Code/Bounce  
  LGPL3
* ClickEncoder
  https://github.com/0xPIT/encoder
  MIT
* CmdMessenger
  http://playground.arduino.cc//Code/CmdMessenger
  MIT
* DallasTemperature
  http://milesburton.com/Dallas_Temperature_Control_Library   
  GPL2 
* EasyVr
	https://github.com/RoboTech-srl/EasyVR-Arduino
	MIT
* EEPROMex
  http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino/
  LGPL
* LiquidCrystal440
  https://code.google.com/p/liquidcrystal440/
  GPL3
* MultiMap
  http://playground.arduino.cc/Main/MultiMap
  MIT?
* OneWire
  http://www.pjrc.com/teensy/td_libs_OneWire.html
  CUSTOM
* PID
  http://playground.arduino.cc/Code/PIDLibrary
  GPL3
* PinChangeInt
  http://arduino.cc/playground/Main/PinChangeInt
  GPL3
* RunningAverage
  http://playground.arduino.cc/Main/runningAverage
  MIT?
* TimerOne
  http://arduino.cc/playground/Code/Timer1
  GPL3
* WiFly
	https://github.com/Seeed-Studio/WiFi_Shield
	MIT?
