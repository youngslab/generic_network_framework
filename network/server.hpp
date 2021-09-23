

#pragma once

#include <unordered_map>
#include "channel.hpp"

#include <iostream>
#include <fmt/format.h>

namespace gnf {

template <typename Socket, typename MessageType> class Connections {

  using ChannelType = Channel<Socket, MessageType>;
  using ChannelCreatedHandlerType =
      std::function<void(std::unique_ptr<ChannelType>)>;

  boost::asio::io_context &_asioContext;
  boost::asio::ip::tcp::acceptor _asioAcceptor;
  ChannelCreatedHandlerType _onChannelCreated;

  auto startAccepting() -> void {

		std::cout << "start accepting clients\n";

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
  Connections(boost::asio::io_context &context)
      : _asioContext(context), _asioAcceptor(context) {}

  auto start(int port, ChannelCreatedHandlerType handler) {

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

    _asioAcceptor.open(endpoint.protocol());
    _asioAcceptor.set_option(
	boost::asio::ip::tcp::acceptor::reuse_address(true));
    _asioAcceptor.bind(endpoint);
    _asioAcceptor.listen();

    _onChannelCreated = handler;


    startAccepting();
  }

  auto stop() {
    _asioAcceptor.cancel();
    _onChannelCreated = nullptr;
  }

  ~Connections() { stop(); }
};

/*
 * Resposibility
 * - manage sessions.
 * - create threads.
 * */
template <typename Socket, typename MessageType> class GenericServer {
private:
  // threads
  int _threadCount;
  std::vector<std::thread> _threadPool;

  // asio for a server
  boost::asio::io_service _asioContext;

  // Connections
  int _id;
  Connections<Socket, MessageType> connections;
  std::unordered_map<uint32_t, std::unique_ptr<Channel<Socket, MessageType>>>
      sessions;

  auto onConnected(std::unique_ptr<Channel<Socket, MessageType>> channel) {
    auto id = _id++;
		std::cout << fmt::format("channel created {}\n", id);
    sessions[id] = std::move(channel);
    sessions[id]->registerOnMessageRecieved(
	[=](auto const &msg) { onMessageRecieved(id, msg); });
    sessions[id]->start();
  }

  virtual auto onSessionCreated(int id) -> void {}

  virtual auto onMessageRecieved(int id, Message<MessageType> const &msg)
      -> void {}

public:
  GenericServer(uint32_t port, int threadCount)
      : connections(_asioContext), _threadCount(threadCount) {

    connections.start(port,
		      [=](auto channel) { onConnected(std::move(channel)); });

    // start pool of threads to process the asio events
    for (int i = 0; i < _threadCount; ++i) {
      _threadPool.emplace_back([=] { _asioContext.run(); });
    }
  }

  virtual ~GenericServer() {
    for (auto &t : _threadPool) {
      if (!t.joinable())
	continue;
      t.join();
    }
  }
};

} // namespace gnf
