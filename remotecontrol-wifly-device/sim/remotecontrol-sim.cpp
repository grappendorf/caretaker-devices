#include <Simulator.h>
#include <Arduino.h>
#include <Bounce.h>
#include <stdlib.h>
#include "../src/remotecontrol.h"

class RemoteControlSimulator : public Simulator {
  friend class RemoteControlArduinoHandler;
  friend class RemoteControlBounceHandler;
private:
  bool button[4];
  bool buttonNew[4];
  bool buttonFalling[4];
  bool buttonRising[4];

  void initDeviceState() {
    DeviceState state;
    for (int i = 0; i < 4; ++i) {
      button[i] = false;
      buttonNew[i] = false;
      buttonFalling[i] = false;
      buttonRising[i] = false;
      std::string name = "button" + std::to_string(i);
      state[name] = "0";
    }
    sendDeviceState(state);
  }

  void updateDeviceState(DeviceState &state){
    for (int i = 0; i < 4; ++i) {
      std::string name = "button" + std::to_string(i);
      if (state.find(name) != state.end()) {
        buttonNew[i] = state[name] == "1";
      }
    }
  }

  int pinToIndex(uint8_t pin) {
    switch(pin) {
      case BUTTON_01_PIN:
        return 0;
      case BUTTON_02_PIN:
        return 1;
      case BUTTON_03_PIN:
        return 2;
      case BUTTON_04_PIN:
        return 3;
      }
    return -1;
  }
};

RemoteControlSimulator sim;

class RemoteControlBounceHandler : public BounceHandler {
  int update(uint8_t pin) {
    int i = sim.pinToIndex(pin);
    if (i >= 0) {
      if (sim.button[i] != sim.buttonNew[i]) {
        sim.buttonFalling[i] = sim.buttonNew[i] == false;
        sim.buttonRising[i] = sim.buttonNew[i] == true;
        sim.button[i] = sim.buttonNew[i];
        return 1;
      } else {
        sim.buttonFalling[i] = false;
        sim.buttonRising[i] = false;
      }
    }
    return 0;
  }

  bool fallingEdge(uint8_t pin) {
    int i = sim.pinToIndex(pin);
    if (i >= 0) {
      return sim.buttonFalling[i];
    }
    return false;
  }

  bool risingEdge(uint8_t pin) {
    int i = sim.pinToIndex(pin);
    if (i >= 0) {
      return sim.buttonRising[i];
    }
    return false;
  }

  int read(uint8_t pin) {
    int i = sim.pinToIndex(pin);
    if (i >= 0) {
      return sim.button[i] ? LOW : HIGH;
    }
    return HIGH;
  }
};

int main(int argc, char* argv[]) {
  RemoteControlBounceHandler bounceHandler;
  sim.setBounceHandler(&bounceHandler);
  sim.setDeviceType(DEVICE_TYPE);
  return sim.run(argc, argv);
}
