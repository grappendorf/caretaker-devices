#include <Simulator.h>
#include <Arduino.h>
#include <stdlib.h>
#include "../src/switch8.h"

class Switch8Simulator : public Simulator {
  friend class Switch8ArduinoHandler;
private:
  bool relais[8];

  void initDeviceState() {
    DeviceState state;
    for (int i = 0; i < 8; ++i) {
      std::string key = "relais" + std::to_string(i);
      state[key] = "0";
    }
    sendDeviceState(state);
  }

  void updateDeviceState(DeviceState &state) {
    for (int i = 0; i < 8; ++i) {
      std::string key = "relais" + std::to_string(i);
      if (state.find(key) != state.end()) {
        relais[i] = state[key] == "1";
      }
    }
  }

  int pinToIndex(uint8_t pin) {
    switch(pin) {
      case PIN_SWITCH0:
        return 0;
      case PIN_SWITCH1:
        return 1;
      case PIN_SWITCH2:
        return 2;
      case PIN_SWITCH3:
        return 3;
      case PIN_SWITCH4:
        return 4;
      case PIN_SWITCH5:
        return 5;
      case PIN_SWITCH6:
        return 6;
      case PIN_SWITCH7:
        return 7;
      }
    return -1;
  }
};

Switch8Simulator sim;

class Switch8ArduinoHandler : public ArduinoHandler {
  uint8_t digitalRead(uint8_t pin) {
    int index = sim.pinToIndex(pin);
    if (index >= 0) {
      return sim.relais[index] ? HIGH : LOW;
    }
    return HIGH;
  }

  void digitalWrite(uint8_t pin, uint8_t value) {
    int index = sim.pinToIndex(pin);
    if (index >= 0) {
      std::string key = "relais" + std::to_string(index);
      sim.relais[index] = value == HIGH;
      sim.sendDeviceState(key, sim.relais[index] ? "1" : "0");
    }
  }
};

int main(int argc, char* argv[]) {
  Switch8ArduinoHandler arduinoHandler;
  sim.setArduinoHandler(&arduinoHandler);
  sim.setDeviceType(DEVICE_TYPE);
  return sim.run(argc, argv);
}
