#include <fmt/format.h>
#include <iostream>

#include <gnf/server.hpp>
#include <gnf/uds.hpp>

#include "chat_common.hpp"

#ifdef UDS
using Protocol = boost::asio::local::stream_protocol;
#else
using Protocol = boost::asio::ip::tcp;
#endif

class ChatServer : public gnf::GenericServer<Protocol, ChatMessageType> {

  using Server = gnf::GenericServer<Protocol, ChatMessageType>;

private:
  std::unordered_map<uint32_t, std::vector<uint32_t>> fds;

protected:
  virtual auto onMessageRecieved(int id,
                                 gnf::Message<ChatMessageType> const &msg)
      -> void override {

    if (msg.header.type == ChatMessageType::FD) {
      if (msg.control.size() < 1)
        throw std::runtime_error("Failed to get a file descriptor");

      int fd;
      // dummy msghdr is for using CMGG Macros
      struct msghdr dummy = {0};
      dummy.msg_control = (void *)msg.control.data(); // WARN: it's constant
      dummy.msg_controllen = CMSG_SPACE(sizeof(fd));

      ::cmsghdr *cmsg = CMSG_FIRSTHDR(&dummy);
      unsigned char *data = CMSG_DATA(cmsg);
      memmove(&fd, data, sizeof(fd));
      // show a contents of the fd.
      std::cout << fmt::format("{}) fd recieved. \n", id);
      print(fd);

    } else if (msg.header.type == ChatMessageType::Message) {
      std::cout << fmt::format("{}) message recieved. msg={}\n", id,
                               msg.body.data());
    }

    Server::onMessageRecieved(id, msg);
  }

  virtual auto onMessageSent(int id, const gnf::Message<ChatMessageType> &msg)
      -> void override {
    Server::onSessionCreated(id);
    std::cout << fmt::format("{}) message sent. msg={}\n", id, msg.body.data());
  }

  virtual auto onSessionCreated(int id) -> void override {
    Server::onSessionCreated(id);
    std::cout << fmt::format("{}) session created.\n", id);
  }

  virtual auto onSessionClosed(int id, std::error_code const &ec)
      -> void override {
    Server::onSessionClosed(id, ec);
    std::cout << fmt::format("{}) session closed. ec={}\n", id, ec.message());
  }

public:
#ifdef UDS
  auto start(std::string const &filepath) {
    unlink(filepath.data());
    EndpointType endpoint(filepath);
    Server::start(endpoint);
  }
#else
  auto start(int port) {
    EndpointType endpoint(Protocol::v4(), port);
    Server::start(endpoint);
  }
#endif
};

int main() {
  ChatServer server;
#ifdef UDS
  server.start(SOCKET_PATH);
#else
  server.start(PORT);
#endif
  while (1) {
  }
}
