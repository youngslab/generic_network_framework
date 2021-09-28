
#pragma once

#include <bits/stdint-uintn.h>
#include <vector>
#include <sys/socket.h>
#include <sys/un.h>

namespace gnf {

template <typename T> struct Message {
  struct Header {
    T type;
    uint32_t bodylen;
    uint32_t controllen;
  };

  Header header;
  std::vector<uint8_t> body;
  std::vector<uint8_t> control;

  auto size() { return sizeof(Header) + body.size(); }
};

} // namespace gnf
