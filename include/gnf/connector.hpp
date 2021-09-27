
#pragma once

#include "channel.hpp"

namespace gnf {

template <typename Protocol, typename MessageType> class Connector {
public:
  using AcceptorType = get_acceptor_t<Protocol>;
  using EndpointType = get_endpoint_t<Protocol>;
  using SocketType = get_socket_t<Protocol>;
  using ChannelType = Channel<Protocol, MessageType>;
  using ChannelCreatedHandlerType =
      std::function<void(std::unique_ptr<ChannelType>)>;

  Connector(boost::asio::io_context &context, ChannelCreatedHandlerType handler)
      : _asioContext(context), _asioAcceptor(context),
	_onChannelCreated(handler) {}

  ~Connector() { stop(); }

  auto start(EndpointType const &endpoint) {
    _asioAcceptor.open(endpoint.protocol());
    //_asioAcceptor.set_option(
    // AcceptorType::reuse_address(true));
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
  ChannelCreatedHandlerType _onChannelCreated;
  AcceptorType _asioAcceptor;

  auto startAccepting() -> void {
    auto socket = std::make_unique<SocketType>(_asioContext);
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
