

#pragma once

#include <unordered_map>
#include "channel.hpp"

#include <iostream>
#include <fmt/format.h>

namespace gnf {

template <typename Socket, typename MessageType> class Connector {

  using ChannelType = Channel<Socket, MessageType>;
  using ChannelCreatedHandlerType =
      std::function<void(std::unique_ptr<ChannelType>)>;

  boost::asio::io_context &_asioContext;
  boost::asio::ip::tcp::acceptor _asioAcceptor;
  ChannelCreatedHandlerType _onChannelCreated;

  auto startAccepting() -> void {
    auto socket = std::make_unique<Socket>(_asioContext);
    // unique_ptr will be moved to labmda. So we keep it as ref for a moment.
    auto &ref = *socket;
    _asioAcceptor.async_accept(ref, [=,
				     s = std::move(socket)](auto &ec) mutable {
      if (this->_onChannelCreated)
	this->_onChannelCreated(
	    std::make_unique<ChannelType>(this->_asioContext, std::move(s)));
      this->startAccepting();
    });
  }

public:
  Connector(boost::asio::io_context &context, ChannelCreatedHandlerType handler)
      : _asioContext(context), _asioAcceptor(context),
	_onChannelCreated(handler) {}

  auto start(int port) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

    _asioAcceptor.open(endpoint.protocol());
    _asioAcceptor.set_option(
	boost::asio::ip::tcp::acceptor::reuse_address(true));
    _asioAcceptor.bind(endpoint);
    _asioAcceptor.listen();

    startAccepting();
  }

  auto stop() {
    _asioAcceptor.cancel();
    _onChannelCreated = nullptr;
  }

  ~Connector() { stop(); }
};

/*
 * Resposibility
 * - manage sessions.
 * - create threads.
 * */
template <typename Socket, typename MessageType> class GenericServer {
private:
  boost::asio::io_service _asioContext;

  uint32_t _numWorkers;
  std::vector<std::thread> _workers;

  // Connector
  Connector<Socket, MessageType> _connector;

  // Sessions
  int _id;
  std::unordered_map<uint32_t, std::unique_ptr<Channel<Socket, MessageType>>>
      sessions;

  auto onConnected(std::unique_ptr<Channel<Socket, MessageType>> channel) {
    auto id = _id++;

    channel->registerOnMessageRecieved(
	[=](auto const &msg) { onMessageRecieved(id, msg); });

    channel->registerOnMessageSent(
	[=](auto const &msg) { onMessageSent(id, msg); });

    channel->start();

    sessions[id] = std::move(channel);

    onSessionCreated(id);
  }

  virtual auto onSessionCreated(int id) -> void {}

  virtual auto onMessageRecieved(int id, Message<MessageType> const &msg)
      -> void {}

  virtual auto onMessageSent(int id, Message<MessageType> const &msg)
      -> void {}

public:
  GenericServer(int numWorkers=1)
      : _numWorkers(numWorkers), _id(0), _connector(_asioContext, [=](auto channel) {
	  onConnected(std::move(channel));
	}) {}

  auto start(uint32_t port) {

    _connector.start(port);

    for (int i = 0; i < _numWorkers; ++i) {
      _workers.emplace_back([=] { _asioContext.run(); });
    }
  }

  virtual ~GenericServer() {
    for (auto &worker : _workers) {
      if (!worker.joinable())
	continue;
      worker.join();
    }
  }
};

} // namespace gnf
