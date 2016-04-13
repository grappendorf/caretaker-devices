#include <Simulator.h>
#include <Arduino.h>
#include <Bounce.h>
#include <stdlib.h>
#include "../src/switch.h"

class SwitchSimulator : public Simulator {
  friend class SwitchArduinoHandler;
  friend class SwitchBounceHandler;
private:
  bool relais;
  bool button; bool buttonNew;
  bool buttonFalling;

  void initDeviceState() {
    relais = false;
    button = false;
    buttonNew = false;
    buttonFalling = false;
    DeviceState state;
    state["relais"] = "0";
    state["button"] = "0";
    sendDeviceState(state);
  }

  void updateDeviceState(DeviceState &state) {
    if (state.find("button") != state.end()) {
      buttonNew = state["button"] == "1";
    }
  }
};

SwitchSimulator sim;

class SwitchArduinoHandler : public ArduinoHandler {
  uint8_t digitalRead(uint8_t pin) {
    if (pin == SWITCH_PIN) {
      return sim.relais ? HIGH : LOW;
    }
    return HIGH;
  }

  void digitalWrite(uint8_t pin, uint8_t value) {
    if (pin == SWITCH_PIN) {
      sim.relais = value == HIGH;
      sim.sendDeviceState("relais", sim.relais ? "1" : "0");
    }
  }
};

class SwitchBounceHandler : public BounceHandler {
  int update(uint8_t pin) {
    if (sim.button != sim.buttonNew) {
      sim.buttonFalling = sim.buttonNew == false;
      sim.button = sim.buttonNew;
      return 1;
    } else {
      sim.buttonFalling = false;
    }
    return 0;
  }

  bool fallingEdge(uint8_t pin) {
    return sim.buttonFalling;
  }
};

int main(int argc, char* argv[]) {
  SwitchArduinoHandler arduinoHandler;
  sim.setArduinoHandler(&arduinoHandler);
  SwitchBounceHandler bounceHandler;
  sim.setBounceHandler(&bounceHandler);
  sim.setDeviceType(DEVICE_TYPE);
  return sim.run(argc, argv);
}
