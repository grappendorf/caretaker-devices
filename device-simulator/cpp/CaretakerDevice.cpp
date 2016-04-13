#include <CaretakerDevice.h>
#include <Simulator.h>

static Stream stream;
static CmdMessenger messenger(stream);
static unsigned long register_with_server_timeout = 0;
static bool isOperational = false;
static DeviceDescriptor *device;
const int REGISTER_TIMEOUT_MS = 5 * 1000;
void onServerRegisterResponse();

void deviceInit(DeviceDescriptor& descriptor) {
  device = &descriptor;
  descriptor.messenger = &messenger;
  if (descriptor.registerMessageHandlers) {
    (*descriptor.registerMessageHandlers)();
  }
  messenger.printLfCr(true);
  messenger.attach(MSG_REGISTER_RESPONSE, onServerRegisterResponse);
}

void deviceUpdate() {
  messenger.feedinSerialData();
  if (! isOperational) {
    if (Simulator::getInstance()->getCurrentMillis() > register_with_server_timeout) {
      Simulator::getInstance()->log("Sending registration request to server");
      messenger.sendCmdStart(MSG_REGISTER_REQUEST);
      messenger.sendCmdArg(Simulator::getInstance()->getDeviceId().c_str());
      messenger.sendCmdArg(device->type);
      messenger.sendCmdArg(Simulator::getInstance()->getDeviceName().c_str());
      messenger.sendCmdArg(device->description);
      if (device->sendServerRegisterParams) {
        (*device->sendServerRegisterParams)();
      }
      messenger.sendCmdEnd();
      register_with_server_timeout = Simulator::getInstance()->getCurrentMillis() + REGISTER_TIMEOUT_MS;
    }
  }
}

bool deviceIsOperational() {
  return isOperational;
}

void onServerRegisterResponse() {
  Simulator::getInstance()->log("Received registration response from server");
  isOperational = true;
}
