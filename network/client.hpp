
#pragma once

#include "channel.hpp"

#include <iostream>
#include <fmt/format.h>

namespace gnf {

template <typename Socket, typename MessageType> class GenericClient {

private:
  boost::asio::io_service _asioContext;
  std::unique_ptr<Channel<Socket, MessageType>> _channel;
  std::thread worker;
  virtual auto onMessageRecieved(Message<MessageType> const &message) -> void {}

  virtual auto onConnected(std::unique_ptr<Socket> socket) -> void {
		//on connected
    _channel = std::make_unique<Channel<Socket, MessageType>>(
	_asioContext, std::move(socket));

    _channel->registerOnMessageRecieved(
	[=](auto const &msg) { onMessageRecieved(msg); });

    _channel->start();
  }

public:
  GenericClient() {}

  auto start(std::string const &ip, int port) -> void {
		std::cout << fmt::format("client start. ip={}, port={}\n", ip, port); 

    // endpoint setup
    boost::asio::ip::tcp::endpoint endpoint(
	boost::asio::ip::address::from_string(ip), port);

    // make socket
    auto socket = std::make_unique<Socket>(_asioContext);

    auto &ref = *socket;
    ref.async_connect(endpoint,
		      [me = this, s = std::move(socket)](auto &ec) mutable {
					std::cout <<"On Connected\n";
			me->onConnected(std::move(s));
		      });

    // run a worker
    worker = std::thread([=]() { _asioContext.run(); });
  }

  auto sendAsync(Message<MessageType> const &m) -> void {
		std::cout << "send a message\n";
    _channel->sendAsync(m);
  }
};
} // namespace gnf
