cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

set(CMAKE_TOOLCHAIN_FILE $ENV{ARDUINO_CMAKE}/cmake/ArduinoToolchain.cmake)
set(ARDUINO_1_5 True)

project(caretaker-device-reflowoven C CXX)

link_directories($ENV{ARDUINO_LIBRARIES})
link_directories(${ARDUINO_SDK_PATH}/libraries)
link_directories(${ARDUINO_SDK_PATH}/hardware/arduino/avr/libraries)

generate_arduino_firmware(
	reflowoven
	SRCS ../caretaker-device/device.cpp reflowoven.cpp
	HDRS ../caretaker-device/device.h
	BOARD uno
	PROGRAMMER dragon
	PORT USB)
