#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdio.h>

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define HIGH 0x1
#define LOW 0x0

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
uint8_t digitalRead(uint8_t pin);
unsigned long millis();
void delay(unsigned long ms);

class Simulator;

class ArduinoHandler {
public:
  ArduinoHandler()  {}
  void setSimulator(Simulator *simulator);
  virtual void digitalWrite(uint8_t pin, uint8_t value) {}
  virtual uint8_t digitalRead(uint8_t pin) {return LOW;}
protected:
  Simulator *simulator;
};


#endif // ARDUINO_H
