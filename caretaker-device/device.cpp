/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#include <Arduino.h>
#include <WiFi_Shield/WiFly.h>
#include <EEPROMEx/EEPROMex.h>
#include <CmdMessenger/CmdMessenger.h>
#include "device.h"

#ifdef DEBUG
int debugLastState = -1;
SoftwareSerial debug(2, 3);
#endif

#define MAGIC_NUMBER 0xCAFE

#define DEVICE_UUID_LEN 36
#define DEVICE_NAME_MAX_LEN 32
#define SSID_MAX_LEN 32
#define PHRASE_MAX_LEN 64

uint16_t magic;
char deviceUuid[DEVICE_UUID_LEN + 1];
char deviceName[DEVICE_NAME_MAX_LEN + 1];
char ssid[SSID_MAX_LEN + 1];
char phrase[PHRASE_MAX_LEN + 1];

#define EEPROM_SIZE EEPROMSizeATmega328

#define EEPROM_MAGIC_ADDR         0
#define EEPROM_DEVICE_UUID_ADDR   (EEPROM_MAGIC_ADDR + sizeof(magic))
#define EEPROM_DEVICE_NAME_ADDR   (EEPROM_DEVICE_UUID_ADDR + sizeof(deviceUuid))
#define EEPROM_SSID_ADDR          (EEPROM_DEVICE_NAME_ADDR + sizeof(deviceName))
#define EEPROM_PHRASE_ADDR        (EEPROM_SSID_ADDR + sizeof(ssid))

static WiFly wifly(Serial);

static CmdMessenger messenger = CmdMessenger(wifly);

static DeviceDescriptor* device;

#define BUF_LEN 128
static char buf[BUF_LEN + 1];
#define SERVER_ADDRESS_LEN 15
static char serverAddress[SERVER_ADDRESS_LEN + 1];
static unsigned long blinkMillis;
static int blinkIndex;
static int* blinkPattern;
#ifdef AUTO_CONFIG
#define MAC_LEN 17
static char mac[MAC_LEN + 1];
static int newDeviceBlinkPattern[] = { 1500, 100, 0 };
static int discoveredBlinkPattern[] = { 500, 500, 0 };
static int factoryResetBlinkPattern[] = { 50, 50, 0 };
#endif

#define WAIT_FOR_FACTORYRESET_TIMEOUT (3L * 1000L)
#define WAIT_FOR_CONFIG_TIMEOUT (5 * 60L * 1000L)
#define WAIT_FOR_REGISTRATION_TIMEOUT (20L * 1000L)
#define PING_INTERVAL (5 * 60 * 1000L)

static unsigned long timeoutMillis;
static unsigned long nextPingMillis;

enum State {
  STATE_INIT,
  STATE_CONNECT_WLAN,
  STATE_WAIT_FOR_BROADCAST_RESPONSE,
  STATE_REGISTER_WITH_SERVER,
  STATE_WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE,
  STATE_OPERATIONAL,
#ifdef AUTO_CONFIG
  STATE_NEW_DEVICE,
  STATE_WAIT_FOR_DISCOVERY,
  STATE_SEND_INFO,
  STATE_WAIT_FOR_SEND_INFO_ACK,
  STATE_WAIT_FOR_CONFIG,
  STATE_CONFIGURE_DEVICE,
  STATE_CONFIG_TIMEOUT,
  STATE_FACTORY_RESET_CONFIRM,
  STATE_FACTORY_RESET
#endif
};

static State state = STATE_INIT;

void onServerRegisterResponse();
void onPing();

#ifdef DEBUG
void dumpConfigValues() {
  debug.print(F("- UUID: "));
  debug.println(deviceUuid);
  debug.print(F("- Name: "));
  debug.println(deviceName);
  debug.print(F("- Type: "));
  debug.println(device->type);
  debug.print(F("- SSID: "));
  debug.println(ssid);
  debug.print(F("- WPA2: "));
  debug.println(phrase);
}
#endif

/**
 * Check if the currently received wifly data matches the specified string.
 *
 * @param s The string to check
 * @return True if s matches the received data
 */
bool checkWiflyInput(const char* s) {
  int len = strlen(s);
  if (wifly.available() < len) {
    return false;
  }
  uint8_t i = 0;
  for (; i < len; ++i) {
    if (wifly.read() != s[i]) {
      return false;
    }
  }
  return true;
}

/**
 * Read a '\n' terminated line from the wifly module.
 *
 * @param res Where to store the received data
 * @param maxChars Maximum number of characters to read
 * @return True if a line was read successfully
 */
boolean wiflyReadline(char* res, int maxChars) {
  int readBytes = wifly.readBytesUntil('\n', buf, BUF_LEN);
  if (readBytes == 0) {
    return false;
  }
  if (readBytes > maxChars) {
    readBytes = maxChars;
  }
  buf[readBytes] = '\0';
  strcpy(res, buf);
  return true;
}

/**
 * Activate a given LED blink pattern.
 *
 * @param _blink_pattern The pattern to activate
 */
void activateBlinkPattern(int* _blinkPattern) {
  blinkPattern = _blinkPattern;
  if (device->ledPin > 0) {
    digitalWrite(device->ledPin, LOW);
    if (blinkPattern) {
      blinkIndex = 0;
      blinkMillis = millis() + blinkPattern[0];
    }
  }
}

/**
 * Initialize the device.
 *
 * @param descriptor Device information
 */
void deviceInit(DeviceDescriptor& descriptor) {
#ifdef DEBUG
  debug.begin(9600);
#endif

  EEPROM.setMemPool(0, EEPROM_SIZE);

  device = &descriptor;
  device->messenger = & messenger;

  if (device->ledPin > 0) {
    pinMode(device->ledPin, OUTPUT);
    digitalWrite(device->ledPin, LOW);
  }
  blinkPattern = NULL;

  if (device->buttonPin > 0) {
    pinMode(device->buttonPin, INPUT);
    digitalWrite(device->buttonPin, HIGH);
  }

  messenger.attach(MSG_REGISTER_RESPONSE, onServerRegisterResponse);
  messenger.attach(MSG_PING, onPing);
  if (device->registerMessageHandlers) {
    (*device->registerMessageHandlers)();
  }

  Serial.begin(57600);
}

/**
 * This method must be called regularily within the loop() function
 */
void deviceUpdate() {
  // Blink the LED

  if (device->ledPin > 0) {
    if (blinkPattern && millis() > blinkMillis) {
      digitalWrite(device->ledPin, digitalRead(device->ledPin) == HIGH ? LOW : HIGH);
      ++blinkIndex;
      if (blinkPattern[blinkIndex] == 0) {
        blinkIndex = 0;
      }
      blinkMillis = millis() + blinkPattern[blinkIndex];
    }
  }

  // State machine

  switch (state) {

    case STATE_INIT:
      // --------------------------------------------------------------------------------
      // Initial state after powering up

      DEBUG_PRINTLN_STATE(F("INIT"))

#ifdef AUTO_CONFIG
      if (device->buttonPin > 0) {
        if (digitalRead(device->buttonPin) == LOW) {
          activateBlinkPattern(factoryResetBlinkPattern);
          timeoutMillis = millis() + WAIT_FOR_FACTORYRESET_TIMEOUT;
          state = STATE_FACTORY_RESET_CONFIRM;
          break;
        }
      }
#endif

      magic = EEPROM.readInt(EEPROM_MAGIC_ADDR);
      if (magic != MAGIC_NUMBER) {
#ifdef AUTO_CONFIG
        state = STATE_NEW_DEVICE;
#else
        state = STATE_INIT;
#endif
        break;
      }

      DEBUG_PRINTLN(F("Valid EEPROM Data"))
      EEPROM.readBlock(EEPROM_DEVICE_UUID_ADDR, deviceUuid, DEVICE_UUID_LEN);
      EEPROM.readBlock(EEPROM_DEVICE_NAME_ADDR, deviceName, DEVICE_NAME_MAX_LEN);
      EEPROM.readBlock(EEPROM_SSID_ADDR, ssid, SSID_MAX_LEN);
      EEPROM.readBlock(EEPROM_PHRASE_ADDR, phrase, PHRASE_MAX_LEN);
      DEBUG_DUMP_CONFIG_VALUES()
      state = STATE_CONNECT_WLAN;
      break;

#ifdef AUTO_CONFIG
    case STATE_FACTORY_RESET_CONFIRM:
      // --------------------------------------------------------------------------------
      // User must press the button WAIT_FOR_FACTORYRESET_TIMEOUT seconds
      // before the factory reset is actually executed

      DEBUG_PRINTLN_STATE(F("FACTORY_RESET_CONFIRM"))
      if (device->buttonPin > 0) {
        if (digitalRead(device->buttonPin) == HIGH) {
          // Factory reset was cancelled
          delay(10);
          activateBlinkPattern(NULL);
          state = STATE_INIT;
        } else if (millis() > timeoutMillis) {
          state = STATE_FACTORY_RESET;
        }
      }
      break;
#endif

#ifdef AUTO_CONFIG
    case STATE_FACTORY_RESET:
      // --------------------------------------------------------------------------------
      // Perform a factory reset

      DEBUG_PRINTLN_STATE(F("FACTORY_RESET"))
      for (int i = 0; i < EEPROM_SIZE; ++i) {
        EEPROM.writeByte(i, 0xff);
      }
      activateBlinkPattern(NULL);
      state = STATE_INIT;
      break;
#endif

#ifdef AUTO_CONFIG
    case STATE_NEW_DEVICE:
      // --------------------------------------------------------------------------------
      // Entered if this is a new device with no stored valid EEPROM data

      DEBUG_PRINTLN_STATE(F("NEW_DEVICE"))
      activateBlinkPattern(newDeviceBlinkPattern);
      wifly.reset();
      wifly.sendCommand("set u b 57600\r");
      wifly.sendCommand("get m\r", "Mac Addr=");
      wifly.receive((uint8_t *) mac, 17);
      mac[17] = '\0';
      DEBUG_PRINT(F("- MAC: "))
      DEBUG_PRINTLN(mac)
      DEBUG_PRINTLN(F("- Activate AP mode"))
      wifly.sendCommand("set w j 7\r", "OK"); // Enable AP mode
      wifly.sendCommand("set w c 6\r", "OK");
      sprintf(buf, "set a s Caretaker-%c%c%c%c%c%c%c%c%c%c%c%c\r", mac[0], mac[1], mac[3], mac[4], mac[6], mac[7],
          mac[9], mac[10], mac[12], mac[13], mac[15], mac[16]);
      wifly.sendCommand(buf, "OK");
      wifly.sendCommand("set a p 0347342d\r", "OK");
      wifly.sendCommand("set i d 4\r", "OK"); // Enable DHCP server
      wifly.sendCommand("set i a 192.168.0.1\r", "OK");
      wifly.sendCommand("set i n 255.255.255.0\r", "OK");
      wifly.sendCommand("set i g 192.168.0.1\r", "OK");
      wifly.save();
      wifly.reboot();
      state = STATE_WAIT_FOR_DISCOVERY;
      break;
#endif

#ifdef AUTO_CONFIG
    case STATE_WAIT_FOR_DISCOVERY:
      // --------------------------------------------------------------------------------
      // Wait until the configuration app has opened a connection to this device

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_DISCOVERY"))
      if (checkWiflyInput("*OPEN*")) {
        activateBlinkPattern(discoveredBlinkPattern);
        state = STATE_SEND_INFO;
      }
      break;
#endif

#ifdef AUTO_CONFIG
    case STATE_SEND_INFO:
      // --------------------------------------------------------------------------------
      // Send information about this device to the configuration app

      DEBUG_PRINTLN_STATE(F("SEND_INFO"))
      wifly.println(); // Other side receives "*HELLO*\n"
      wifly.println(mac);
      wifly.println(device->type);
      wifly.println(device->description);
      state = STATE_WAIT_FOR_SEND_INFO_ACK;
      break;
#endif

#ifdef AUTO_CONFIG
    case STATE_WAIT_FOR_SEND_INFO_ACK:
      // --------------------------------------------------------------------------------
      // Wait for the receiving confirmation

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_SEND_INFO_ACK"))
      if (checkWiflyInput("*CLOS*")) {
        timeoutMillis = millis() + WAIT_FOR_CONFIG_TIMEOUT;
        state = STATE_WAIT_FOR_CONFIG;
      }
      break;
#endif

#ifdef AUTO_CONFIG
    case STATE_WAIT_FOR_CONFIG:
      // --------------------------------------------------------------------------------
      // Configuration app has WAIT_FOR_CONFIG_TIMEOUT seconds to send the configuration

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_CONFIG"))
      if (checkWiflyInput("*OPEN*")) {
        state = STATE_CONFIGURE_DEVICE;
      } else if (millis() > timeoutMillis) {
        state = STATE_CONFIG_TIMEOUT;
      }
      break;
#endif

#ifdef AUTO_CONFIG
    case STATE_CONFIG_TIMEOUT:
      // --------------------------------------------------------------------------------
      // No configuration received, restart discovery process

      DEBUG_PRINTLN_STATE(F("CONFIG_TIMEOUT"))
      state = STATE_NEW_DEVICE;
      break;
#endif

#ifdef AUTO_CONFIG
    case STATE_CONFIGURE_DEVICE:
      // --------------------------------------------------------------------------------
      // Read and store the confifuration values

      DEBUG_PRINTLN_STATE(F("CONFIGURE_DEVICE"))
      wifly.println(); // Other side receives "*HELLO*\n"
      if (!(wiflyReadline(deviceUuid, DEVICE_UUID_LEN))) {
        state = STATE_CONFIG_TIMEOUT;
        break;
      }
#ifdef DEBUG
      // Always use the same uuid for debugging
      strcpy(deviceUuid, "89dd4596-3356-4c7a-9876-e3af6ea68e15");
#endif
      if (!(wiflyReadline(deviceName, DEVICE_NAME_MAX_LEN))) {
        state = STATE_CONFIG_TIMEOUT;
        break;
      }
      if (!(wiflyReadline(ssid, SSID_MAX_LEN))) {
        state = STATE_CONFIG_TIMEOUT;
        break;
      }
      if (!(wiflyReadline(phrase, PHRASE_MAX_LEN))) {
        state = STATE_CONFIG_TIMEOUT;
        break;
      }

      while (!checkWiflyInput("*CLOS*")) {
      }

      DEBUG_DUMP_CONFIG_VALUES()

      EEPROM.writeInt(EEPROM_MAGIC_ADDR, MAGIC_NUMBER);
      EEPROM.writeBlock(EEPROM_DEVICE_UUID_ADDR, deviceUuid, DEVICE_UUID_LEN);
      EEPROM.writeBlock(EEPROM_DEVICE_NAME_ADDR, deviceName, DEVICE_NAME_MAX_LEN);
      EEPROM.writeBlock(EEPROM_SSID_ADDR, ssid, SSID_MAX_LEN);
      EEPROM.writeBlock(EEPROM_PHRASE_ADDR, phrase, PHRASE_MAX_LEN);

      activateBlinkPattern(NULL);
      state = STATE_CONNECT_WLAN;
      break;
#endif

    case STATE_CONNECT_WLAN:
      // --------------------------------------------------------------------------------
      // Connect to the configured WLAN and start sending broadcast messages

      DEBUG_PRINTLN_STATE(F("CONNECT_WLAN"))
      wifly.reset();
      wifly.sendCommand("set u b 57600\r");
      wifly.sendCommand("set i h 0.0.0.0\r", "OK"); // UDP auto pairing
      wifly.sendCommand("set i f 0x40\r", "OK"); // UDP auto pairing
      wifly.sendCommand("set i d 1\r", "OK"); // DHCP client on
      wifly.sendCommand("set i p 1\r", "OK"); // Use UDP
      wifly.sendCommand("set b i 7\r", "OK"); // UDP broadcast interval 8 secs
#ifdef BROADCAST_PORT
      wifly.sendCommand("set b p 44444\r", "OK"); // Set broadcast port to 44444 when debugging
#endif
      wifly.sendCommand("set w a 4\r", "OK");
      wifly.sendCommand("set w c 0\r", "OK");
      wifly.sendCommand("set w j 1\r", "OK");
      snprintf(buf, BUF_LEN, "set w s %s\r", ssid);
      wifly.sendCommand(buf, "OK");
      snprintf(buf, BUF_LEN, "set w p %s\r", phrase);
      wifly.sendCommand(buf, "OK");
      snprintf(buf, BUF_LEN, "set o d %s\r", deviceName);
      wifly.sendCommand(buf, "OK");
      wifly.save();
      wifly.reboot();
      state = STATE_WAIT_FOR_BROADCAST_RESPONSE;
      break;

    case STATE_WAIT_FOR_BROADCAST_RESPONSE:
      // --------------------------------------------------------------------------------
      // Wait until the broadcast receiver sends a broadcast response with the actual
      // with server IP address

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_BROADCAST_RESPONSE"))
      if (checkWiflyInput("*SERVER*\n")) {
        if (wiflyReadline(serverAddress, SERVER_ADDRESS_LEN)) {
          snprintf(buf, BUF_LEN, "- Broadcast response from server: %s", serverAddress);
          DEBUG_PRINTLN(buf);
          // Set the server IP address for UDP transmissions
          // Disable UDP broadcast
          snprintf(buf, BUF_LEN, "set i h %s\r", serverAddress);
          wifly.sendCommand(buf, "OK");
          wifly.sendCommand("set b i 0\r", "OK");
          wifly.dataMode();
          state = STATE_REGISTER_WITH_SERVER;
        }
      }
      break;

    case STATE_REGISTER_WITH_SERVER:
      // --------------------------------------------------------------------------------
      // Send a registration request to the server, containing all device information:

      DEBUG_PRINTLN_STATE(F("REGISTER_WITH_SERVER"))
      messenger.sendCmdStart(MSG_REGISTER_REQUEST);
      messenger.sendCmdArg(deviceUuid);
      messenger.sendCmdArg(device->type);
      messenger.sendCmdArg(deviceName);
      messenger.sendCmdArg(device->description);
      if (device->sendServerRegisterParams) {
        (*device->sendServerRegisterParams)();
      }
      messenger.sendCmdEnd();
      timeoutMillis = millis() + WAIT_FOR_REGISTRATION_TIMEOUT;
      state = STATE_WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE;
      break;

    case STATE_WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE:
      // --------------------------------------------------------------------------------
      // Wait until the server responds to our registration request

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE"))
      if (millis() > timeoutMillis) {
        state = STATE_REGISTER_WITH_SERVER;
      } else {
        messenger.feedinSerialData();
      }
      break;

    case STATE_OPERATIONAL:
      // --------------------------------------------------------------------------------
      // Normal operational mode

      DEBUG_PRINTLN_STATE(F("OPERATIONAL"))
      if (millis () > nextPingMillis) {
        messenger.sendCmd(MSG_PING);
        nextPingMillis = millis() + PING_INTERVAL;
      }
      messenger.feedinSerialData();
      break;
  }
}

/**
 * This message is sent by the server as a response to a MSG_REGISTER_REQUEST.
 */
void onServerRegisterResponse() {
  DEBUG_PRINTLN(F("* ServerRegisterResponse"))
  if (state == STATE_WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE) {
    state = STATE_OPERATIONAL;
  }
}

/**
 * Respond a server ping with a ping.
 */
void onPing() {
  DEBUG_PRINTLN(F("* Ping"))
  messenger.sendCmd(MSG_PING);
}

/**
 * Return true if the device is in STATE_OPERATIONAL.
 */
bool deviceIsOperational() {
  return state == STATE_OPERATIONAL;
}

void deviceWiflyFlush() {
  wifly.flush();
}
