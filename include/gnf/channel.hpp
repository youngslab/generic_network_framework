#pragma once

#include "message.hpp"
#include <fmt/format.h>
#include <functional>
#include <deque>
#include <boost/asio.hpp>
#include <iostream>

namespace gnf {

template <typename Socket, typename MessageType>
class Channel
    : public std::enable_shared_from_this<Channel<Socket, MessageType>> {
public:
  /* A Server and a Client have resposibility to make and passs the socket which
   * is completed to make a connection between them */
  Channel(boost::asio::io_context &context, std::unique_ptr<Socket> socket)
      : _socket(std::move(socket)), _writeStrand(context) {}

  virtual ~Channel() {}

  auto start() { readHeaderAsync(); }

  auto stop() {
    _socket->close();
    _socket->release();
  }

  auto sendAsync(Message<MessageType> const &m) -> void {
    auto msg = std::make_shared<Message<MessageType>>(m);

    writeHeaderAsync(msg);

    if (m.header.size != 0) {
      writeBodyAsync(msg);
    }

    onMessageSentAsync(msg);
  }

  auto recieveAsync() -> void {
    // TODO: make a resposibility to trigger next recived operation to a User
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

  auto registerOnClosed(std::function<void(std::error_code const &ec)> onClosed)
      -> void {
    _onClosed = onClosed;
  }

private:
  boost::asio::io_context _context;
  std::unique_ptr<Socket> _socket;

  std::function<void(std::error_code const &)> _onClosed;

  // read
  Message<MessageType> _readMessage;
  std::function<void(Message<MessageType> const &)> _onMessageRecieved;

  // write
  boost::asio::io_service::strand _writeStrand;
  std::function<void(Message<MessageType> const &)> _onMessageSent;

  auto onMessageHeaderRead(typename Message<MessageType>::Header const &header)
      -> void {
    if (header.size != 0) {
      readBodyAysnc(header.size);
      return;
    }
    onMessageRecieved();
  }

  auto onMessageRecieved() -> void {
    if (_onMessageRecieved)
      _onMessageRecieved(_readMessage);
    start();
  }

  auto onMessageSentAsync(std::shared_ptr<Message<MessageType>> const &message)
      -> void {
    if (_onMessageSent)
      _onMessageSent(*message);
  }

  auto readHeaderAsync() -> void {
    boost::asio::async_read(
	*_socket,
	boost::asio::buffer(&_readMessage.header,
			    sizeof(typename Message<MessageType>::Header)),
	[&](std::error_code ec, std::size_t size) {
	  if (ec) {
	    if (_onClosed) {
	      _onClosed(ec);
	      return;
	    }
	    throw std::runtime_error(fmt::format(
		"Failed to read a header of the message. ec={}, size={}",
		ec.message(), size));
	  }
	  onMessageHeaderRead(_readMessage.header);
	});
  }

  auto readBodyAysnc(std::size_t len) -> void {
    // async read a body
    _readMessage.body.resize(len);
    boost::asio::async_read(
	*_socket, boost::asio::buffer(_readMessage.body.data(), len),
	[&](std::error_code ec, std::size_t size) {
	  if (ec) {
	    if (_onClosed) {
	      _onClosed(ec);
	      return;
	    }
	    throw std::runtime_error(fmt::format(
		"Failed to read a body of the message. ec={}, size={}",
		ec.message(), size));
	  }
	  onMessageRecieved();
	});
  }

  auto writeHeaderAsync(std::shared_ptr<Message<MessageType>> message) {
    // write asynchronosely
    auto ptr = &(message->header);
    boost::asio::async_write(
	*_socket,
	boost::asio::buffer(ptr, sizeof(typename Message<MessageType>::Header)),
	_writeStrand.wrap([=](auto &ec, auto size) {
	  if (ec) {
	    if (_onClosed) {
	      _onClosed(ec);
	      return;
	    }
	    throw std::runtime_error(fmt::format(
		"Failed to send a header of the message. ec={}, size={}",
		ec.message(), size));
	  }
	}));
  }

  auto writeBodyAsync(std::shared_ptr<Message<MessageType>> message) -> void {
    boost::asio::async_write(
	*_socket,
	boost::asio::buffer(message->body.data(), message->header.size),
	_writeStrand.wrap([=](auto &ec, auto size) {
	  if (ec) {
	    if (_onClosed) {
	      _onClosed(ec);
	      return;
	    }
	    throw std::runtime_error(fmt::format(
		"Failed to send a body of the message. ec={}, size={}",
		ec.message(), size));
	  }
	}));
  }
};

} // namespace gnf
