
#pragma once

#include <bits/stdint-uintn.h>
#include <vector>

namespace gnf {

template <typename T> struct Message {
  struct Header {
    T type;
    uint32_t size;
  };

  Header header;
  std::vector<uint8_t> body;

  auto size() { return sizeof(Header) + body.size(); }
};

} // namespace gnf
