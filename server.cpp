#include <iostream>
#include <fmt/format.h>

#include "network/server.hpp"
#include "common.hpp"

class ChatServer
    : public gnf::GenericServer<boost::asio::ip::tcp::socket, ChatMessageType> {

  void onMessageRecieved(int id,
			 const gnf::Message<ChatMessageType> &msg) override {
    gnf::GenericServer<boost::asio::ip::tcp::socket,
		       ChatMessageType>::onMessageRecieved(id, msg);
    std::cout << fmt::format("{}) message={}\n", id, msg.body.data());
  }

  void onMessageSent(int id,
		     const gnf::Message<ChatMessageType> &msg) override {
    std::cout << fmt::format("{}) message sent.\n", id);
  }

  void onSessionCreated(int id) override {
    std::cout << fmt::format("{}) session created.\n", id);
  }

public:
  using gnf::GenericServer<boost::asio::ip::tcp::socket,
			   ChatMessageType>::GenericServer;
};

int main() {
  ChatServer server;
  server.start(8765);
  while (1) {
  }
}
