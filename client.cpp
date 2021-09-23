
#include "network/client.hpp"
#include "common.hpp"
#include <iostream>
#include <sstream>

class ChatClient
    : public gnf::GenericClient<boost::asio::ip::tcp::socket, ChatMessageType> {

public:
  // using parent's constructor
  using gnf::GenericClient<boost::asio::ip::tcp::socket,
			   ChatMessageType>::GenericClient;
  using Client =
      gnf::GenericClient<boost::asio::ip::tcp::socket, ChatMessageType>;

protected:
  auto onDisconnected(const std::error_code &ec) -> void override {
    Client::onDisconnected(ec);
    std::cout << "server disconnected\n";
  }
  auto onMessageRecieved(const gnf::Message<ChatMessageType> &message)
      -> void override {
    Client::onMessageRecieved(message);
    std::cout << "message recieved\n";
  }

  auto onMessageSent(const gnf::Message<ChatMessageType> &message)
      -> void override {
    Client::onMessageSent(message);
    std::cout << fmt::format("message sent. body={}\n", message.body.data());
  }
};

int main() {

  ChatClient client;
  client.start(IP, PORT);

  while (1) {
    auto buff = std::array<char, 1000>{};
    std::cin.getline(buff.data(), buff.size());

    if (strcmp(buff.data(), "q") == 0) {
      std::cout << "Client exited.\n";
      break;
    }

    auto len = strlen(buff.data()) + 1;
    gnf::Message<ChatMessageType> message;
    message.header.type = ChatMessageType::Message;
    message.header.size = len;
    message.body.resize(len);
    message.body.insert(message.body.begin(), buff.data(), buff.data() + len);

    if (!client.isConnected())
      break;

    client.sendAsync(message);
  }

  return 0;
}
