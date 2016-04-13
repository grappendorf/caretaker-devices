#ifndef CARETAKER_DEVICE_H
#define CARETAKER_DEVICE_H

#include <CmdMessenger.h>
#include <../../caretaker-device/src/messages.h>

typedef struct _DeviceDescriptor {
  const char* type;
  const char* description;
  int ledPin;
  int buttonPin;
  void (*registerMessageHandlers)();
  void (*sendServerRegisterParams)();
  void (*operationalCallback)();
  CmdMessenger* messenger;
} DeviceDescriptor;

void deviceInit(DeviceDescriptor& descriptor);
void deviceUpdate();
bool deviceIsOperational();

#endif // CARETAKER_DEVICE_H
