
#include <boost/asio/local/stream_protocol.hpp>
#include <gnf/client.hpp>
#include <iostream>
#include <sstream>

#include "chat_common.hpp"

#ifdef UDS
using Protocol = boost::asio::local::stream_protocol;
#else
using Protocol = boost::asio::ip::tcp;
#endif

class ChatClient : public gnf::GenericClient<Protocol, ChatMessageType> {

public:
  // using parent's constructor
  using gnf::GenericClient<Protocol, ChatMessageType>::GenericClient;
  using Client = gnf::GenericClient<Protocol, ChatMessageType>;

#ifdef UDS
  auto start(std::string filepath) {
    // unlink(filepath.data());
    EndpointType endpoint(filepath.data());
    Client::start(endpoint);
  }

#else
  auto start(std::string const &ip, int port) {
    EndpointType endpoint(boost::asio::ip::address::from_string(ip), port);
    Client::start(endpoint);
  }
#endif

protected:
  auto onDisconnected(const std::error_code &ec) -> void override {
    Client::onDisconnected(ec);
    std::cout << fmt::format("server disconnected. ec={} \n", ec.message());
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

auto makeControlMessage(int fd) -> std::vector<uint8_t> {
  std::vector<uint8_t> buff(CMSG_SPACE(sizeof(fd)));
  struct msghdr msg = {0};
  msg.msg_control = buff.data();

  // update cmsghdr
  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

  return buff;
}

int main() {

  ChatClient client;
#ifdef UDS
  client.start(SOCKET_PATH);
#else
  client.start(IP, PORT);
#endif

  int fd = open("foo", O_RDONLY);
  if (fd < 0) {
    std::cout << "File not found.\n";
    return -1;
  }

  while (1) {
    auto buff = std::array<char, 1000>{};
    std::cin.getline(buff.data(), buff.size());

    if (strcmp(buff.data(), "q") == 0) {
      std::cout << "Client exited.\n";
      break;
    }

    gnf::Message<ChatMessageType> message;

    if (strcmp(buff.data(), "fd") == 0) {
      auto len = strlen(buff.data()) + 1;

      gnf::Message<ChatMessageType> message;
      message.header.type = ChatMessageType::FD;
      message.header.bodylen = 0;
      message.control = makeControlMessage(fd);
      message.header.controllen = message.control.size();

    } else {

      auto len = strlen(buff.data()) + 1;
      message.header.type = ChatMessageType::Message;
      message.header.bodylen = len;
      message.body.resize(len);
      message.body.insert(message.body.begin(), buff.data(), buff.data() + len);
    }

    if (!client.isConnected())
      break;

    client.sendAsync(message);
  }

  return 0;
}
