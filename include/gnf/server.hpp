

#pragma once

#include "connector.hpp"
#include <unordered_map>

namespace gnf {

/**
 * @brief GenericServer will manage sessions which contain id and channel.
 *
 * @tparam Socket
 * @tparam MessageType
 */
template <typename Protocol, typename MessageType> class GenericServer {

public:
  using EndpointType = get_endpoint_t<Protocol>;

  GenericServer(int numWorkers = 1)
      : _numWorkers(numWorkers), _id(0),
	_connector(_asioContext,
		   [=](auto channel) { onConnected(std::move(channel)); }) {}

  auto start(EndpointType const &endpoint) -> void {

    _connector.start(endpoint);

    for (int i = 0; i < _numWorkers; ++i) {
      _workers.emplace_back([=] { _asioContext.run(); });
    }
  }

  auto stop() {

    _asioContext.post([=]() { _connector.stop(); });

    for (auto &worker : _workers) {
      if (!worker.joinable())
	continue;
      worker.join();
    }
  }

  virtual ~GenericServer() { stop(); }

protected:
  /**
   * @brief Callback funcntion. It will be called when session is created,
   *
   * @param id Session id
   *
   * @return Nothing
   */
  virtual auto onSessionCreated(int id) -> void {}

  virtual auto onSessionClosed(int id, std::error_code const &ec) -> void {
    sessions.erase(id);
  }

  virtual auto onMessageRecieved(int id, Message<MessageType> const &msg)
      -> void {}

  virtual auto onMessageSent(int id, Message<MessageType> const &msg) -> void {}

private:
  // asio context
  boost::asio::io_service _asioContext;

  // workers
  uint32_t _numWorkers;
  std::vector<std::thread> _workers;

  // Connector
  Connector<Protocol, MessageType> _connector;

  // Sessions
  int _id;
  std::unordered_map<uint32_t, std::unique_ptr<Channel<Protocol, MessageType>>>
      sessions;

  auto onConnected(std::unique_ptr<Channel<Protocol, MessageType>> channel) {
    auto id = _id++;

    // Register handlers
    channel->registerOnMessageRecieved(
	[=](auto const &msg) { onMessageRecieved(id, msg); });

    channel->registerOnMessageSent(
	[=](auto const &msg) { onMessageSent(id, msg); });

    channel->registerOnClosed([=](auto const &ec) { onSessionClosed(id, ec); });

    // Start listening
    channel->start();

    // Resigster internal session's container
    sessions[id] = std::move(channel);

    onSessionCreated(id);
  }
};

} // namespace gnf
