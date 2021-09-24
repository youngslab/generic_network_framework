
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
    auto buff = std::array<char, 1000>{};
    std::cin.getline(buff.data(), buff.size());

    if (strcmp(buff.data(), "q") == 0) {
      std::cout << "Client exited.\n";
      break;
    }

    auto len = strlen(buff.data());
    gnf::Message<ChatMessageType> message;
    message.header.type = ChatMessageType::Message;
    message.header.size = len;
    message.body.resize(len);
    message.body.insert(message.body.begin(), buff.data(), buff.data() + len);

    std::cout << fmt::format("send data: {}\n", message.body.data());

    client.sendAsync(message);
  }
}
