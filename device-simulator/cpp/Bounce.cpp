#include <Bounce.h>
#include <Simulator.h>

Bounce::Bounce(uint8_t pin, unsigned long interval_millis) {
  this->pin = pin;
}

int Bounce::update() {
  if (Simulator::getBounceHandler() != NULL) {
    return Simulator::getBounceHandler()->update(pin);
  }
  return 0;
}

int Bounce::read() {
  if (Simulator::getBounceHandler() != NULL) {
    return Simulator::getBounceHandler()->read(pin);
  }
  return HIGH;
}

bool Bounce::risingEdge() {
  if (Simulator::getBounceHandler() != NULL) {
    return Simulator::getBounceHandler()->risingEdge(pin);
  }
  return false;
}

bool Bounce::fallingEdge() {
  if (Simulator::getBounceHandler() != NULL) {
    return Simulator::getBounceHandler()->fallingEdge(pin);
  }
  return false;
}
