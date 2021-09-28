
#pragma once

#include <boost/asio.hpp>

static const std::string SOCKET_PATH = "/home/jaeyoungs/chat_socket";

static const int PORT = 8876;

static const char *IP = "127.0.0.1";

enum class ChatMessageType { Message, FD };

#define MAX_BUFSIZE 255

auto print(int fd) {
  auto fp = fdopen(fd, "r");
  while (1) {
    char buf[MAX_BUFSIZE];
    while (1) {
      fgets(buf, MAX_BUFSIZE, fp);
      if (feof(fp)) {
	return;
      }
      printf("%s", buf); //한 라인을 얻어와서 콘솔 화면에 출력
    }
  }
}
