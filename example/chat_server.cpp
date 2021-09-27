#include <iostream>
#include <fmt/format.h>

#include <gnf/server.hpp>
#include "chat_common.hpp"

#ifdef UDS
using Protocol = boost::asio::local::stream_protocol;
#else
using Protocol = boost::asio::ip::tcp;
#endif

class ChatServer : public gnf::GenericServer<Protocol, ChatMessageType> {

  using Server = gnf::GenericServer<Protocol, ChatMessageType>;

protected:
  virtual auto onMessageRecieved(int id,
				 gnf::Message<ChatMessageType> const &msg)
      -> void override {
    Server::onMessageRecieved(id, msg);
    std::cout << fmt::format("{}) message recieved. msg={}\n", id,
			     msg.body.data());
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
