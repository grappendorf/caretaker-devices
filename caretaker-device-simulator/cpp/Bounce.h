#ifndef BOUNCE_H
#define BOUNCE_H

#include <Arduino.h>

class Bounce {
public:
  Bounce(uint8_t pin, unsigned long interval_millis);
  int update();
  int read();
  bool risingEdge();
	bool fallingEdge();
private:
  uint8_t pin;
};

class Simulator;

class BounceHandler {
public:
  BounceHandler()  {}
  void setSimulator(Simulator *simulator);
  virtual int update(uint8_t pin) {}
  virtual int read(uint8_t pin) {}
  virtual bool risingEdge(uint8_t pin) {return false;}
  virtual bool fallingEdge(uint8_t pin) {return false;}
  Simulator *simulator;
};

#endif // BOUNCE_H
