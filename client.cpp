
#include "network/client.hpp"
#include "common.hpp"
#include <iostream>
#include <sstream>

class ChatClient
    : public gnf::GenericClient<boost::asio::ip::tcp::socket, ChatMessageType> {

protected:
  auto onMessageRecieved(const gnf::Message<ChatMessageType> &message)
      -> void override {
    gnf::GenericClient<boost::asio::ip::tcp::socket,
		       ChatMessageType>::onMessageRecieved(message);
    std::cout << "message recieved\n";
  }

  auto onMessageSent(const gnf::Message<ChatMessageType> &message)
      -> void override {
    gnf::GenericClient<boost::asio::ip::tcp::socket,
		       ChatMessageType>::onMessageSent(message);
    std::cout << "message sent\n";
  }

public:
  using gnf::GenericClient<boost::asio::ip::tcp::socket,
			   ChatMessageType>::GenericClient;
};

int main() {

  ChatClient client;
  client.start("127.0.0.1", 8765);
  while (1) {
    getchar();

    gnf::Message<ChatMessageType> message;
    message.header.type = ChatMessageType::Message;
    message.header.size = 0;
    client.sendAsync(message);
  }
}
