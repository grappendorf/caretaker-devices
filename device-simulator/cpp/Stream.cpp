#include <Stream.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <Simulator.h>

#define min(a,b) (((a)<(b))?(a):(b))

int Stream::available() {
  return Simulator::getInstance()->getMessageFromServer().length();
}

size_t Stream::read() {
  if (available() == 0) {
    return 0;
  }
  std::string &msg = Simulator::getInstance()->getMessageFromServer();
  int c = msg[0];
  msg.erase(0, 1);
  return 1;
}

size_t Stream::readBytes(char *buffer, size_t length) {
  std::string &msg = Simulator::getInstance()->getMessageFromServer();
  int len = min(length, msg.length());
  strncpy(buffer, msg.c_str(), len);
  msg.erase(0, len);
  return len;
}

size_t Stream::print(char c) {
  buffer += c;
  return 1;
}

size_t Stream::print(int i) {
  std::string s = std::to_string(i);
  buffer += s;
  return s.length();
}

size_t Stream::print(const char *s) {
  buffer += s;
  return strlen(s);
}

size_t Stream::println() {
  Simulator::getInstance()->sendMessageToServer(buffer);
  buffer.clear();
  return 1;
}
