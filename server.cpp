#include <iostream>
#include <fmt/format.h>

#include "network/server.hpp"
#include "common.hpp"

class ChatServer
    : public gnf::GenericServer<boost::asio::ip::tcp::socket, ChatMessageType> {

  using Server =
      gnf::GenericServer<boost::asio::ip::tcp::socket, ChatMessageType>;

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
};

int main() {
  ChatServer server;
  server.start(PORT);
  while (1) {
  }
}
