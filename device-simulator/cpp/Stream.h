#ifndef STREAM_H
#define STREAM_H

#include <stdlib.h>
#include <string>

class Stream {
public:
  int available();
  size_t read();
  size_t readBytes(char *buffer, size_t length);
  size_t print(char c);
  size_t print(int i);
  size_t print(const char *);
  size_t println();
private:
  std::string buffer;
};

#endif // STREAM_H
