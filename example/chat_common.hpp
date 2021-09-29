
#pragma once

#include <boost/asio.hpp>

static const std::string SOCKET_PATH = "/home/jaeyoung/chat_socket";

static const int PORT = 8876;

static const char *IP = "127.0.0.1";

enum class ChatMessageType { Message, FD };

#define MAX_BUFSIZE 255

auto to_fd(std::vector<uint8_t> const &ctrlmsg) -> int {

  int fd;
  // dummy header to access control data.
  struct msghdr dummy = {0};
  dummy.msg_control = (void *)ctrlmsg.data(); // WARN: it's constant
  dummy.msg_controllen = CMSG_SPACE(sizeof(fd));

  ::cmsghdr *cmsg = CMSG_FIRSTHDR(&dummy);
  unsigned char *data = CMSG_DATA(cmsg);
  memmove(&fd, data, sizeof(fd));

  return fd;
}

auto to_control_message(int fd) -> std::vector<uint8_t> {
  std::vector<uint8_t> buff(CMSG_SPACE(sizeof(fd)));

  struct msghdr msg = {0};
  msg.msg_control = buff.data();
  msg.msg_controllen = CMSG_SPACE(sizeof(fd));

  cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;

  // update data
  memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));

  return buff;
}

auto to_string(int fd) -> std::string {
  if (fd < 0)
    return "";

  struct stat sb;
  fstat(fd, &sb);

  std::string res;
  res.resize(sb.st_size);

  read(fd, const_cast<char *>(res.data()), sb.st_size);
  return res;
}

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
