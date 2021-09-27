
#pragma once

#include "channel.hpp"

#include <iostream>
#include <fmt/format.h>

namespace gnf {

template <typename Protocol, typename MessageType> class GenericClient {

  using SocketType = get_socket_t<Protocol>;
  using EndpointType = get_endpoint_t<Protocol>;

private:
  boost::asio::io_service _asioContext;
  std::unique_ptr<Channel<Protocol, MessageType>> _channel;
  std::thread _worker;

  bool _isConnected;

protected:
  virtual auto onMessageRecieved(Message<MessageType> const &message) -> void {}

  virtual auto onMessageSent(Message<MessageType> const &message) -> void {}

  virtual auto onDisconnected(std::error_code const &ec) -> void {
    _isConnected = false;
  }

  virtual auto onConnected(std::unique_ptr<SocketType> socket) -> void {
    _isConnected = true;

    // on connected
    _channel = std::make_unique<Channel<Protocol, MessageType>>(
	_asioContext, std::move(socket));

    _channel->registerOnMessageRecieved(
	[=](auto const &msg) { onMessageRecieved(msg); });

    _channel->registerOnMessageSent(
	[=](auto const &msg) { onMessageSent(msg); });

    _channel->registerOnClosed([=](auto const &err) { onDisconnected(err); });

    _channel->start();
  }

public:
  GenericClient() : _isConnected(false) {}

  virtual ~GenericClient() {
    // stop
    _asioContext.post([=]() { _channel->stop(); });

    // join
    if (_worker.joinable()) {
      _worker.join();
    }
  }

  auto start(EndpointType const &endpoint) -> void {
    // make socket
    auto socket = std::make_unique<SocketType>(_asioContext);

    auto &ref = *socket;
    ref.async_connect(endpoint,
		      [me = this, s = std::move(socket)](auto &ec) mutable {
			me->onConnected(std::move(s));
		      });
    // run a worker
    _worker = std::thread([=]() { _asioContext.run(); });
  }

  auto sendAsync(Message<MessageType> const &m) -> void {
    _channel->sendAsync(m);
  }

  auto isConnected() -> bool { return _isConnected; }
};
} // namespace gnf
