#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <string>
#include <map>
#include <netinet/in.h>

class BounceHandler;
class ArduinoHandler;

class Simulator {
public:
  Simulator();
  static Simulator *getInstance();
  int run(int argc, char* argv[]);
  void log(const char *format, ...);
  void setBounceHandler(BounceHandler *bounceHandler);
  static BounceHandler *getBounceHandler();
  void setArduinoHandler(ArduinoHandler *arduinoHandler);
  static ArduinoHandler *getArduinoHandler();
  void sendMessageToServer(std::string &msg);
  std::string &getMessageFromServer();
  std::string getDeviceId();
  std::string getDeviceName();
  void setDeviceType(std::string deviceType);
  unsigned long getCurrentMillis();

protected:
  typedef std::map<std::string, std::string> DeviceState;
  void sendDeviceState(DeviceState &state);
  void sendDeviceState(std::string key, std::string value);

private:
  static void onSigInt(int signo);
  static void onRedisConnected(const redisAsyncContext *redis, int status);
  static void onRedisDisconnected(const redisAsyncContext *redis, int status);
  static void onDeviceStateChanged(redisAsyncContext *redis, void *reply, void *data);
  static void onDeviceStateReceived(redisAsyncContext *redis, void *reply, void *data);
  static void onTick(int fd, short event, void *arg);
  static void onDeviceKeyDeleted(redisAsyncContext *redis, void *reply, void *data);
  static void onMessageReceivedFromServer(int fd, short event, void *arg);
  void parseProgramArgs(int argc, char* argv[]);
  void initEvents();
  void initNetwork();
  void connectToRedis();
  void disconnectFromRedis();
  void tick();
  void nextTick();
  virtual void initDeviceState() {}
  virtual void updateDeviceState(DeviceState &state) {}
  static Simulator *instance;
  static BounceHandler *bounceHandler;
  static ArduinoHandler *arduinoHandler;
  static const int LOOP_INTERVAL_MS = 10;
  std::string deviceId;
  std::string deviceName;
  redisAsyncContext *redis;
  redisAsyncContext *subRedis;
  bool redisConnected;
  bool subRedisConnected;
  struct event_base *events;
  bool deviceStateChanged;
  struct sockaddr_in serverAddress;
  struct sockaddr_in deviceAddress;
  int msgSocket;
  std::string messageFromServer;
  std::string host;
  int port;
  std::string serverHost;
  int serverPort;
  std::string redisHost;
  int redisPort;
  std::string deviceType;
  unsigned long currentMillis;
};

#endif // SIMULATOR_H
