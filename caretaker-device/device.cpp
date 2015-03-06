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
int debug_last_state = -1;
SoftwareSerial debug(2, 3);
#endif

#define MAGIC_NUMBER 0xCAFE

#define DEVICE_UUID_LEN 36
#define DEVICE_NAME_MAX_LEN 32
#define SSID_MAX_LEN 32
#define PHRASE_MAX_LEN 64

uint16_t magic;
char device_uuid[DEVICE_UUID_LEN + 1];
char device_name[DEVICE_NAME_MAX_LEN + 1];
char ssid[SSID_MAX_LEN + 1];
char phrase[PHRASE_MAX_LEN + 1];

#define EEPROM_SIZE EEPROMSizeATmega328

#define EEPROM_MAGIC_ADDR         0
#define EEPROM_DEVICE_UUID_ADDR   (EEPROM_MAGIC_ADDR + sizeof(magic))
#define EEPROM_DEVICE_NAME_ADDR   (EEPROM_DEVICE_UUID_ADDR + sizeof(device_uuid))
#define EEPROM_SSID_ADDR          (EEPROM_DEVICE_NAME_ADDR + sizeof(device_name))
#define EEPROM_PHRASE_ADDR        (EEPROM_SSID_ADDR + sizeof(ssid))

static WiFly wifly(Serial);

static CmdMessenger messenger = CmdMessenger(wifly);

static DeviceDescriptor* device;

#define BUF_LEN 128
static char buf[BUF_LEN + 1];
#define MAC_LEN 17
static char mac[MAC_LEN + 1];
#define SERVER_ADDRESS_LEN 15
static char server_address[SERVER_ADDRESS_LEN + 1];
static unsigned long blink_millis;
static int blink_index;
static int new_device_blink_pattern[] = { 1500, 100, 0 };
static int discovered_blink_pattern[] = { 500, 500, 0 };
static int factoryreset_blink_pattern[] = { 50, 50, 0 };
static int* blink_pattern;

#define WAIT_FOR_FACTORYRESET_TIMEOUT (3L * 1000L)
#define WAIT_FOR_CONFIG_TIMEOUT (5 * 60L * 1000L)

static unsigned long timeout_millis;

enum State {
  STATE_INIT,
  STATE_NEW_DEVICE,
  STATE_WAIT_FOR_DISCOVERY,
  STATE_SEND_INFO,
  STATE_WAIT_FOR_SEND_INFO_ACK,
  STATE_WAIT_FOR_CONFIG,
  STATE_CONFIGURE_DEVICE,
  STATE_CONFIG_TIMEOUT,
  STATE_CONNECT_WLAN,
  STATE_WAIT_FOR_BROADCAST_RESPONSE,
  STATE_REGISTER_WITH_SERVER,
  STATE_WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE,
  STATE_OPERATIONAL,
  STATE_FACTORY_RESET_CONFIRM,
  STATE_FACTORY_RESET
};

static State state = STATE_INIT;

void onServerRegisterResponse();
void onPingRequest();

#ifdef DEBUG
void dump_config_values() {
  debug.print(F("- UUID: "));
  debug.println(device_uuid);
  debug.print(F("- Name: "));
  debug.println(device_name);
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
bool check_wifly_input(const char* s) {
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
 * @param max_chars Maximum number of characters to read
 * @return True if a line was read successfully
 */
boolean wifly_readline(char* res, int max_chars) {
  int read_bytes = wifly.readBytesUntil('\n', buf, BUF_LEN);
  if (read_bytes == 0) {
    return false;
  }
  if (read_bytes > max_chars) {
    read_bytes = max_chars;
  }
  buf[read_bytes] = '\0';
  strcpy(res, buf);
  return true;
}

/**
 * Activate a given LED blink pattern.
 *
 * @param _blink_pattern The pattern to activate
 */
void activate_blink_pattern(int* _blink_pattern) {
  blink_pattern = _blink_pattern;
  digitalWrite(device->led_pin, LOW);
  if (blink_pattern) {
    blink_index = 0;
    blink_millis = millis() + blink_pattern[0];
  }
}

/**
 * Initialize the device.
 *
 * @param _led_pin Pin to which the status LED is connected
 * @param _button_pin Pin to which the device control button connected
 */
void device_init(DeviceDescriptor& descriptor) {
#ifdef DEBUG
  debug.begin(9600);
#endif

  EEPROM.setMemPool(0, EEPROM_SIZE);

  device = &descriptor;
  device->messenger = & messenger;

  pinMode(device->led_pin, OUTPUT);
  digitalWrite(device->led_pin, LOW);
  blink_pattern = NULL;

  pinMode(device->button_pin, INPUT);
  digitalWrite(device->button_pin, HIGH);

  messenger.attach(MSG_REGISTER_RESPONSE, onServerRegisterResponse);
  messenger.attach(MSG_PING_REQUEST, onPingRequest);
  if (device->register_message_handlers) {
    (*device->register_message_handlers)();
  }

  Serial.begin(57600);
}

/**
 * This method must be called regularily within the loop() function
 */
void device_update() {
  // Blink the LED

  if (blink_pattern && millis() > blink_millis) {
    digitalWrite(device->led_pin, digitalRead(device->led_pin) == HIGH ? LOW : HIGH);
    ++blink_index;
    if (blink_pattern[blink_index] == 0) {
      blink_index = 0;
    }
    blink_millis = millis() + blink_pattern[blink_index];
  }

  // State machine

  switch (state) {

    case STATE_INIT:
      // --------------------------------------------------------------------------------
      // Initial state after powering up

      DEBUG_PRINTLN_STATE(F("INIT"))

      if (digitalRead(device->button_pin) == LOW) {
        activate_blink_pattern(factoryreset_blink_pattern);
        timeout_millis = millis() + WAIT_FOR_FACTORYRESET_TIMEOUT;
        state = STATE_FACTORY_RESET_CONFIRM;
        break;
      }

      magic = EEPROM.readInt(EEPROM_MAGIC_ADDR);
      if (magic != MAGIC_NUMBER) {
        state = STATE_NEW_DEVICE;
        break;
      }

      DEBUG_PRINTLN(F("Valid EEPROM Data"))
      EEPROM.readBlock(EEPROM_DEVICE_UUID_ADDR, device_uuid, DEVICE_UUID_LEN);
      EEPROM.readBlock(EEPROM_DEVICE_NAME_ADDR, device_name, DEVICE_NAME_MAX_LEN);
      EEPROM.readBlock(EEPROM_SSID_ADDR, ssid, SSID_MAX_LEN);
      EEPROM.readBlock(EEPROM_PHRASE_ADDR, phrase, PHRASE_MAX_LEN);
      DEBUG_DUMP_CONFIG_VALUES()
      state = STATE_CONNECT_WLAN;
      break;

    case STATE_FACTORY_RESET_CONFIRM:
      // --------------------------------------------------------------------------------
      // User must press the button WAIT_FOR_FACTORYRESET_TIMEOUT seconds
      // before the factory reset is actually executed

      DEBUG_PRINTLN_STATE(F("FACTORY_RESET_CONFIRM"))
      if (digitalRead(device->button_pin) == HIGH) {
        // Factory reset was cancelled
        delay(10);
        activate_blink_pattern(NULL);
        state = STATE_INIT;
      } else if (millis() > timeout_millis) {
        state = STATE_FACTORY_RESET;
      }
      break;

    case STATE_FACTORY_RESET:
      // --------------------------------------------------------------------------------
      // Perform a factory reset

      DEBUG_PRINTLN_STATE(F("FACTORY_RESET"))
      for (int i = 0; i < EEPROM_SIZE; ++i) {
        EEPROM.writeByte(i, 0xff);
      }
      activate_blink_pattern(NULL);
      state = STATE_INIT;
      break;

    case STATE_NEW_DEVICE:
      // --------------------------------------------------------------------------------
      // Entered if this is a new device with no stored valid EEPROM data

      DEBUG_PRINTLN_STATE(F("NEW_DEVICE"))
      activate_blink_pattern(new_device_blink_pattern);
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

    case STATE_WAIT_FOR_DISCOVERY:
      // --------------------------------------------------------------------------------
      // Wait until the configuration app has opened a connection to this device

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_DISCOVERY"))
      if (check_wifly_input("*OPEN*")) {
        activate_blink_pattern(discovered_blink_pattern);
        state = STATE_SEND_INFO;
      }
      break;

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

    case STATE_WAIT_FOR_SEND_INFO_ACK:
      // --------------------------------------------------------------------------------
      // Wait for the receiving confirmation

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_SEND_INFO_ACK"))
      if (check_wifly_input("*CLOS*")) {
        timeout_millis = millis() + WAIT_FOR_CONFIG_TIMEOUT;
        state = STATE_WAIT_FOR_CONFIG;
      }
      break;

    case STATE_WAIT_FOR_CONFIG:
      // --------------------------------------------------------------------------------
      // Configuration app has WAIT_FOR_CONFIG_TIMEOUT seconds to send the configuration

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_CONFIG"))
      if (check_wifly_input("*OPEN*")) {
        state = STATE_CONFIGURE_DEVICE;
      } else if (millis() > timeout_millis) {
        state = STATE_CONFIG_TIMEOUT;
      }
      break;

    case STATE_CONFIG_TIMEOUT:
      // --------------------------------------------------------------------------------
      // No configuration received, restart discovery process

      DEBUG_PRINTLN_STATE(F("CONFIG_TIMEOUT"))
      state = STATE_NEW_DEVICE;
      break;

    case STATE_CONFIGURE_DEVICE:
      // --------------------------------------------------------------------------------
      // Read and store the confifuration values

      DEBUG_PRINTLN_STATE(F("CONFIGURE_DEVICE"))
      wifly.println(); // Other side receives "*HELLO*\n"
      if (!(wifly_readline(device_uuid, DEVICE_UUID_LEN))) {
        state = STATE_CONFIG_TIMEOUT;
        break;
      }
#ifdef DEBUG
      // Always use the same uuid for debugging
      strcpy(device_uuid, "89dd4596-3356-4c7a-9876-e3af6ea68e15");
#endif
      if (!(wifly_readline(device_name, DEVICE_NAME_MAX_LEN))) {
        state = STATE_CONFIG_TIMEOUT;
        break;
      }
      if (!(wifly_readline(ssid, SSID_MAX_LEN))) {
        state = STATE_CONFIG_TIMEOUT;
        break;
      }
      if (!(wifly_readline(phrase, PHRASE_MAX_LEN))) {
        state = STATE_CONFIG_TIMEOUT;
        break;
      }

      while (!check_wifly_input("*CLOS*")) {
      }

      DEBUG_DUMP_CONFIG_VALUES()

      EEPROM.writeInt(EEPROM_MAGIC_ADDR, MAGIC_NUMBER);
      EEPROM.writeBlock(EEPROM_DEVICE_UUID_ADDR, device_uuid, DEVICE_UUID_LEN);
      EEPROM.writeBlock(EEPROM_DEVICE_NAME_ADDR, device_name, DEVICE_NAME_MAX_LEN);
      EEPROM.writeBlock(EEPROM_SSID_ADDR, ssid, SSID_MAX_LEN);
      EEPROM.writeBlock(EEPROM_PHRASE_ADDR, phrase, PHRASE_MAX_LEN);

      activate_blink_pattern(NULL);
      state = STATE_CONNECT_WLAN;
      break;

    case STATE_CONNECT_WLAN:
      // --------------------------------------------------------------------------------
      // Connect to the configured WLAN and start sending broadcast messages

      DEBUG_PRINTLN_STATE(F("CONNECT_WLAN"))
      wifly.reset();
      wifly.sendCommand("set u b 57600\r");
      snprintf(buf, BUF_LEN, "set o d %s\r", device_name);
      wifly.sendCommand(buf, "OK");
      wifly.sendCommand("set i h 0.0.0.0\r", "OK"); // UDP auto pairing
      wifly.sendCommand("set i f 0x40\r", "OK"); // UDP auto pairing
      wifly.sendCommand("set i d 1\r", "OK"); // DHCP client on
      wifly.sendCommand("set i p 1\r", "OK"); // Use UDP
      wifly.sendCommand("set b i 7\r", "OK"); // UDP broadcast interval 8 secs
      wifly.sendCommand("set w a 4\r", "OK");
      wifly.sendCommand("set w c 0\r", "OK");
      wifly.sendCommand("set w j 1\r", "OK");
      snprintf(buf, BUF_LEN, "set w s %s\r", ssid);
      wifly.sendCommand(buf, "OK");
      snprintf(buf, BUF_LEN, "set w p %s\r", phrase);
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
      if (check_wifly_input("*SERVER*\n")) {
        if (wifly_readline(server_address, SERVER_ADDRESS_LEN)) {
          snprintf(buf, BUF_LEN, "- Broadcast response from server: %s", server_address);
          DEBUG_PRINTLN(buf);
          // Set the server IP address for UDP transmissions
          // Disable UDP broadcast
          snprintf(buf, BUF_LEN, "set i h %s\r", server_address);
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
      messenger.sendCmdArg(device_uuid);
      messenger.sendCmdArg(device->type);
      messenger.sendCmdArg(device_name);
      messenger.sendCmdArg(device->description);
      if (device->send_server_register_params) {
        (*device->send_server_register_params)();
      }
      messenger.sendCmdEnd();
      state = STATE_WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE;
      break;

    case STATE_WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE:
      // --------------------------------------------------------------------------------
      // Wait until the server responds to our registration request
      // TODO: After a timeout we should resend the registration request

      DEBUG_PRINTLN_STATE(F("WAIT_FOR_REGISTER_WITH_SERVER_RESPONSE"))
      messenger.feedinSerialData();
      break;

    case STATE_OPERATIONAL:
      // --------------------------------------------------------------------------------
      // Normal operational mode

      DEBUG_PRINTLN_STATE(F("OPERATIONAL"))
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
 * This message is sent by the server to check the health of a device.
 */
void onPingRequest() {
  DEBUG_PRINTLN(F("* PingRequest"))
  messenger.sendCmd(MSG_PING_RESPONSE);
}

bool device_is_operational() {
  return state == STATE_OPERATIONAL;
}
