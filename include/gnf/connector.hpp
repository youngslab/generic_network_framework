
#pragma once

#include "channel.hpp"

namespace gnf {
template <typename Socket, typename MessageType> class Connector {
public:
  using ChannelType = Channel<Socket, MessageType>;
  using ChannelCreatedHandlerType =
      std::function<void(std::unique_ptr<ChannelType>)>;

  Connector(boost::asio::io_context &context, ChannelCreatedHandlerType handler)
      : _asioContext(context), _asioAcceptor(context),
	_onChannelCreated(handler) {}

  ~Connector() { stop(); }

  auto start(int port) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

    _asioAcceptor.open(endpoint.protocol());
    _asioAcceptor.set_option(
	boost::asio::ip::tcp::acceptor::reuse_address(true));
    // bind
    _asioAcceptor.bind(endpoint);
    _asioAcceptor.listen();

    startAccepting();
  }

  auto stop() {
    _asioAcceptor.cancel();
    _onChannelCreated = nullptr;
  }

private:
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
};
} // namespace gnf
