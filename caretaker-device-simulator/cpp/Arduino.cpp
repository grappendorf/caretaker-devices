#include <Arduino.h>
#include <Simulator.h>

unsigned long millis() {
  return Simulator::getInstance()->getCurrentMillis();
}

void pinMode(uint8_t pin, uint8_t mode) {
}

void digitalWrite(uint8_t pin, uint8_t value) {
  if (Simulator::getArduinoHandler() != NULL) {
    Simulator::getArduinoHandler()->digitalWrite(pin, value);
  }
}

uint8_t digitalRead(uint8_t pin) {
  if (Simulator::getArduinoHandler() != NULL) {
    return Simulator::getArduinoHandler()->digitalRead(pin);
  }
  return LOW;
}
