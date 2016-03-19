#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <hiredis/adapters/libevent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <Simulator.h>

void setup();
void loop();

Simulator *Simulator::instance;
BounceHandler *Simulator::bounceHandler;
ArduinoHandler *Simulator::arduinoHandler;

Simulator::Simulator() {
  instance = this;
  redis = NULL;
  subRedis = NULL;
  redisConnected = false;
  subRedisConnected = false;
  deviceStateChanged = false;
  bounceHandler = NULL;
  msgSocket = -1;
  currentMillis = 0;
}

Simulator *Simulator::getInstance() {
  return instance;
}

int Simulator::run(int argc, char* argv[]) {
  parseProgramArgs(argc, argv);

  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  log("Simulator starting...");
  initEvents();
  initNetwork();
  connectToRedis();
  setup();
  log("Entering event loop");
  event_base_dispatch(events);
  return 0;
}

void Simulator::log(const char *format, ...) {
  va_list args;
  va_start (args, format);
  vprintf (format, args);
  va_end (args);
  printf("\n");
}

void Simulator::setBounceHandler(BounceHandler *bounceHandler) {
  this->bounceHandler = bounceHandler;
}

BounceHandler *Simulator::getBounceHandler() {
  return bounceHandler;
}

void Simulator::setArduinoHandler(ArduinoHandler *arduinoHandler) {
  this->arduinoHandler = arduinoHandler;
}

ArduinoHandler *Simulator::getArduinoHandler() {
  return arduinoHandler;
}

void Simulator::sendMessageToServer(std::string &msg) {
  sendto(msgSocket, msg.c_str(), msg.length(), 0, (struct sockaddr *) &serverAddress, sizeof(struct sockaddr));
}

void Simulator::onSigInt(int signo) {
  instance->log("Terminating...");
  if (instance->events) {
    instance->disconnectFromRedis();
  }
}

std::string &Simulator::getMessageFromServer() {
  return messageFromServer;
}

std::string Simulator::getDeviceId() {
  return deviceId;
}

std::string Simulator::getDeviceName() {
  return deviceName;
}

void Simulator::setDeviceType(std::string deviceType) {
  this->deviceType = deviceType;
}

unsigned long Simulator::getCurrentMillis() {
  return currentMillis;
}

void Simulator::onRedisConnected(const redisAsyncContext *redis, int status) {
  Simulator *self = (Simulator *) redis->data;
  if (status != REDIS_OK) {
    self->log("Error: %s", redis->errstr);
    exit(1);
  }
  if (redis == self->redis) {
    self->log("Redis command connection established");
    self->redisConnected = true;
  }
  if (redis == self->subRedis) {
    self->log("Redis pub/sub connection established");
    self->subRedisConnected = true;
  }
  if (self->redisConnected && self->subRedisConnected) {
    self->initDeviceState();
    self->nextTick();
  }
}

void Simulator::onRedisDisconnected(const redisAsyncContext *redis, int status) {
  Simulator *self = (Simulator *) redis->data;
  if (status != REDIS_OK) {
    self->log("Error: %s", redis->errstr);
    return;
  }
  if (redis == self->redis) {
    self->log("Redis command connection closed");
    self->redis = NULL;
  }
  if (redis == self->subRedis) {
    self->log("Redis pub/sub connection closed");
    self->subRedis = NULL;
  }
  if (self->redis == NULL && self->subRedis == NULL) {
    event_base_loopbreak(self->events);
  }
}

void Simulator::onDeviceStateChanged(redisAsyncContext *redis, void *_reply, void *data) {
  Simulator *self = (Simulator *) redis->data;
  redisReply *reply  = (redisReply *) _reply;
  if (reply == NULL) {
    return;
  }
  if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
    if (strcmp(reply->element[0]->str, "message") == 0) {
      self->deviceStateChanged = true;
    }
  }
}

void Simulator::onDeviceStateReceived(redisAsyncContext *redis, void *_reply, void *data) {
  Simulator *self = (Simulator *) redis->data;
  redisReply *reply  = (redisReply *) _reply;
  if (reply == NULL) {
    return;
  }
  if (reply->type == REDIS_REPLY_ARRAY) {
    std::string msg = "New device state:";
    std::map<std::string, std::string> state;
    for (int i = 0; i < reply->elements; i += 2) {
      std::string key = reply->element[i]->str;
      std::string val = reply->element[i + 1]->str;
      state[key] = val;
      msg += " " + key + "=" + val;
    }
    self->log(msg.c_str());
    self->updateDeviceState(state);
  }
}

void Simulator::onDeviceKeyDeleted(redisAsyncContext *redis, void *reply, void *data) {
  Simulator *self = (Simulator *) redis->data;
  redisAsyncDisconnect(self->subRedis);
  redisAsyncDisconnect(self->redis);
}

void Simulator::onMessageReceivedFromServer(int fd, short event, void *arg) {
  Simulator *self = (Simulator *) arg;
  char buf[256];
  struct sockaddr_in addr;
	socklen_t addrSize = sizeof(addr);
  int len = recvfrom(self->msgSocket, &buf, sizeof(buf) - 1, 0, (struct sockaddr *) &addr, &addrSize);
  buf[len] = '\0';
  self->messageFromServer += buf;
}

void Simulator::onTick(int fd, short event, void *arg) {
  Simulator *self = (Simulator *) arg;
  self->tick();
}

void Simulator::parseProgramArgs(int argc, char* argv[]) {
  host = "localhost";
  port = 2000;
  serverHost = "localhost";
  serverPort = 2000;
  redisHost = "localhost";
  redisPort = 6379;
  static struct option longOptions[] = {
    {"name", required_argument, 0, 'n'},
    {"bind", required_argument, 0, 'b'},
    {"port", required_argument, 0, 'p'},
    {"server", required_argument, 0, 's'},
    {"redis", required_argument, 0, 'r'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  static const char *shortOptions = "n:p:b:s:r:h";
  int c;
  int longIndex = 0;
  while ((c = getopt_long(argc, argv, shortOptions, longOptions, &longIndex)) != -1) {
    switch(c) {
      case 'n':
        deviceName = optarg;
        break;
      case 'p':
        port = std::stoi(optarg);
        break;
      case 'b':
        host = optarg;
        break;
      case 's': {
          serverHost = optarg;
          int colonIndex = serverHost.find(':');
          if (colonIndex != -1) {
            serverPort = std::stoi(serverHost.substr(serverHost.find(':') + 1, -1));
          }
          serverHost = serverHost.substr(0, colonIndex);
          break;
        }
      case 'r' : {
        redisHost = optarg;
        int colonIndex = redisHost.find(':');
        if (colonIndex != -1) {
          redisPort = std::stoi(redisHost.substr(redisHost.find(':') + 1, -1));
        }
        redisHost = redisHost.substr(0, colonIndex);
        break;
      }
      case 'h':
        printf("Usage: %s DEVICE_ID\n", argv[0]);
        printf("  -n, --name=DEVICE_NAME    if not specified, the device id is used as the device name\n");
        printf("  -b, --bind=HOST           listen on ip address HOST (default: localhost)\n");
        printf("  -p, --port=PORT_NUMBER    the port number for UDP communication (default: 2000)\n");
        printf("  -s, --server=SERVER       the server host and (optional) port (default: localhost:2000)\n");
        printf("  -r, --redis=REDIS_SERVER  the host name/ip address of the Redis server (default: localhost:6379)\n");
        printf("  -h, --help                print this help\n");
        exit(0);
      case 0:
        break;
      default:
        printf("»%s --help« prints more help\n", argv[0]);
        exit(1);
    }
  }
  if (optind == argc) {
    printf("%s: Missing device id\n", argv[0]);
    printf("»%s --help« prints more help\n", argv[0]);
    exit(1);
  }
  deviceId = argv[optind];
  if (deviceName.empty()) {
    deviceName = deviceId;
  }
}

void Simulator::initEvents() {
  signal(SIGINT, onSigInt);
  signal(SIGPIPE, SIG_IGN);
  events = event_base_new();
}

void Simulator::initNetwork() {
  msgSocket = socket(PF_INET, SOCK_DGRAM, 0);

  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort);
  struct hostent *hostent;
  hostent = gethostbyname(serverHost.c_str());
  memcpy(&serverAddress.sin_addr, hostent->h_addr, hostent->h_length);

  memset(&deviceAddress, 0, sizeof(deviceAddress));
  deviceAddress.sin_family = AF_INET;
  deviceAddress.sin_port = htons(port);
  inet_aton(host.c_str(), &deviceAddress.sin_addr);

  bind(msgSocket, (struct sockaddr *) &deviceAddress, sizeof(struct sockaddr));

  struct event *socketEvent;
  socketEvent = event_new(events, msgSocket, EV_READ | EV_PERSIST, onMessageReceivedFromServer, this);
  event_add(socketEvent, NULL);
}

void Simulator::connectToRedis() {
  log("Connecting to Redis...");
  redis = redisAsyncConnect(redisHost.c_str(), redisPort);
  if (redis != NULL && redis->err) {
    printf("Error: %s\n", redis->errstr);
    exit(1);
  }
  subRedis = redisAsyncConnect(redisHost.c_str(), redisPort);
  if (subRedis != NULL && subRedis->err) {
    printf("Error: %s\n", subRedis->errstr);
    exit(1);
  }
  redis->data = this;
  subRedis->data = this;
  redisLibeventAttach(redis, events);
  redisLibeventAttach(subRedis, events);
  redisAsyncSetConnectCallback(redis, onRedisConnected);
  redisAsyncSetConnectCallback(subRedis, onRedisConnected);
  redisAsyncSetDisconnectCallback(redis, onRedisDisconnected);
  redisAsyncSetDisconnectCallback(subRedis, onRedisDisconnected);

  redisAsyncCommand(redis, NULL, NULL, "CONFIG set notify-keyspace-events KEA");
  std::string cmd = "HMSET caretaker.devices." + deviceId + " id " + deviceId +
    " type " + deviceType  + " name " + deviceName;
  redisAsyncCommand(redis, NULL, NULL, cmd.c_str());
  cmd = "SUBSCRIBE __keyspace@0__:caretaker.devices." + deviceId;
  redisAsyncCommand(subRedis, onDeviceStateChanged, NULL, cmd.c_str());
}

void Simulator::disconnectFromRedis() {
  log("Disconnect from Redis...");
  std::string cmd = "DEL caretaker.devices." + deviceId;
  redisAsyncCommand(redis, onDeviceKeyDeleted, NULL, cmd.c_str());
}

void Simulator::tick() {
  currentMillis += LOOP_INTERVAL_MS;
  loop();
  if (deviceStateChanged) {
    log("Device state has changed");
    deviceStateChanged = false;
    std::string cmd = "HGETALL caretaker.devices." + deviceId;
    log(("Getting device state with \"" + cmd + "\"").c_str());
    redisAsyncCommand(redis, onDeviceStateReceived, NULL, cmd.c_str());
  }
  nextTick();
}

void Simulator::nextTick() {
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = LOOP_INTERVAL_MS * 1000;
  struct event *ev = event_new(events, 0, EV_TIMEOUT, onTick, this);
  evtimer_add(ev, &tv);
}

void Simulator::sendDeviceState(DeviceState &state) {
  std::string cmd = "HMSET caretaker.devices." + deviceId;
  for (DeviceState::iterator i = state.begin(); i != state.end(); ++i) {
    cmd += " " + i->first + " " + i->second;
  }
  log(("Updating device state with \"" + cmd + "\"").c_str());
  redisAsyncCommand(redis, NULL, NULL, cmd.c_str());
}

void Simulator::sendDeviceState(std::string key, std::string value) {
  std::string cmd = "HMSET caretaker.devices." + deviceId + " " + key + " " + value;
  log(("Updating device state with \"" + cmd + "\"").c_str());
  redisAsyncCommand(redis, NULL, NULL, cmd.c_str());
}
