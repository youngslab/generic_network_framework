

#include <iostream>
#include <fmt/format.h>

#include "network/server.hpp"
#include "common.hpp"

class ChatServer
    : public gnf::GenericServer<boost::asio::ip::tcp::socket, ChatMessageType> {

  void onMessageRecieved(int id,
			 const gnf::Message<ChatMessageType> &msg) override {
    std::cout << fmt::format("{}) recieved msg", id);
  }

  void onSessionCreated(int id) override {
    std::cout << fmt::format("{}) session created", id);
  }

public:
  using gnf::GenericServer<boost::asio::ip::tcp::socket,
			   ChatMessageType>::GenericServer;
};

int main() {
  ChatServer server(8765, 2);

  while (1) {
  }
}
