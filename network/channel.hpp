#pragma once

#include "message.hpp"
#include <functional>
#include <deque>
#include <boost/asio.hpp>
#include <iostream>

namespace gnf {

template <typename Socket, typename MessageType>
class Channel
    : public std::enable_shared_from_this<Channel<Socket, MessageType>> {
private:
  std::unique_ptr<Socket> _socket;

  // read
  Message<MessageType> _readMessage;
  std::function<void(Message<MessageType> const &)> _onMessageRecieved;

  // write
  boost::asio::io_service::strand _writeStrand;
  std::function<void(Message<MessageType> const &)> _onMessageSent;

  auto asyncReadHeader() -> void {
    boost::asio::async_read(
	*_socket,
	boost::asio::buffer(&_readMessage.header,
			    sizeof(typename Message<MessageType>::Header)),
	[&](std::error_code ec, std::size_t length) {
	  if (!ec)
	    onHeaderRead(_readMessage.header);
	});
  }

  auto asyncReadBody(std::size_t len) -> void {
    // async read a body
    boost::asio::async_read(*_socket,
			    boost::asio::buffer(_readMessage.body.data(), len),
			    [&](std::error_code ec, std::size_t length) {
			      if (!ec)
				onCompleted();
			    });
  }

  auto onHeaderRead(typename Message<MessageType>::Header const &header)
      -> void {
    if (header.size != 0) {
      asyncReadBody(header.size);
      return;
    }
    onCompleted();
  }

  auto onCompleted() -> void {
    if (_onMessageRecieved)
      _onMessageRecieved(_readMessage);
    start();
  }

public:
  /* A Server and a Client have resposibility to make and passs the socket which
   * is completed to make a connection between them */
  Channel(boost::asio::io_context &context, std::unique_ptr<Socket> socket)
      : _socket(std::move(socket)), _writeStrand(context) {
    // startRecievingMessage();
  }

  auto start() {
    // request to read a header first
    std::cout << "channel start read\n";
    asyncReadHeader();
  }

  auto sendAsync(Message<MessageType> const &m) -> void {
    // TODO: Support message pool(But, it might need synchronized)
    auto msg = std::make_shared<Message<MessageType>>(m);
    // write asynchronosely
    boost::asio::async_write(
	*_socket, boost::asio::buffer(msg.get(), msg->size()),
	_writeStrand.wrap([me = this, m = msg](auto &ec, auto) {
	  if (me->_onMessageSent)
	    me->_onMessageSent(*m);
	}));
  }

  auto registerOnMessageRecieved(
      std::function<void(Message<MessageType> const &)> onMessageRecieved)
      -> void {
    _onMessageRecieved = onMessageRecieved;
  }

  auto registerOnMessageSent(
      std::function<void(Message<MessageType> const &)> onMessageSent) -> void {
    _onMessageSent = onMessageSent;
  }
};

} // namespace gnf
