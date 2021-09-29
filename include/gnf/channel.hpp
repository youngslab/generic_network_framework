#pragma once

#include "common.hpp"
#include "message.hpp"
#include <boost/asio.hpp>
#include <deque>
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>

namespace gnf {

template <typename Protocol, typename MessageType>
class Channel
    : public std::enable_shared_from_this<Channel<Protocol, MessageType>> {
public:
  using SocketType = get_socket_t<Protocol>;

  /* A Server and a Client have resposibility to make and passs the socket which
   * is completed to make a connection between them */
  Channel(boost::asio::io_context &context, std::unique_ptr<SocketType> socket)
      : _context(context), _socket(std::move(socket)), _writeStrand(context) {}

  virtual ~Channel() {}

  auto start() { readHeaderAsync(); }

  auto stop() {
    _socket->close();
    _socket->release();
  }

  auto sendAsync(Message<MessageType> const &m) -> void {
    auto msg = std::make_shared<Message<MessageType>>(m);

    writeHeaderAsync(msg);

    if (m.header.bodylen != 0) {
      writeBodyAsync(msg);
    }

    if (m.header.controllen != 0) {
      writeControlAsync(msg);
    }

    onMessageSentAsync(msg);
  }

  auto recieveAsync() -> void {
    // TODO: make a resposibility to trigger next recived iperation to a User
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

  auto getNativeHandle() -> uint32_t { return this->_socket->native_handle(); }

private:
  boost::asio::io_context &_context;
  std::unique_ptr<SocketType> _socket;

  std::function<void(std::error_code const &)> _onClosed;

  // read
  Message<MessageType> _readMessage;
  std::function<void(Message<MessageType> const &)> _onMessageRecieved;

  // write
  boost::asio::io_service::strand _writeStrand;
  std::function<void(Message<MessageType> const &)> _onMessageSent;

  auto onMessageHeaderRead(typename Message<MessageType>::Header const &header)
      -> void {

    if (header.bodylen == 0 && header.controllen == 0)
      onMessageRecieved();

    if (header.bodylen != 0) {
      readBodyAysnc(header.bodylen);
      return;
    }

    if (header.controllen != 0) {
      readControlAsync();
      return;
    }
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

  auto readControlAsync() -> void {
    // When recvmsg directly will be fail with "Resouce
    // temporarily unavailable". Becuase it works as an non-blocking mode by
    // default. There is no data to read at the moment. To avoid this, request
    // asio to wait until it is ready to read.
    _socket->async_wait(
        boost::asio::socket_base::wait_read, [=](boost::system::error_code ec) {
          if (ec)
            throw std::runtime_error(fmt::format(
                "Failed to async_wait(wait_read). ec={} \n", ec.message()));

          // dummy data. 0 data size is impossible to send.
          char data;
          iovec io = {.iov_base = &data, .iov_len = 1};

          _readMessage.control.resize(_readMessage.header.controllen);

          msghdr msg = {0};
          msg.msg_iov = &io;
          msg.msg_iovlen = 1;
          msg.msg_control = _readMessage.control.data();
          msg.msg_controllen = _readMessage.header.controllen;

          auto res = recvmsg(_socket->native_handle(), &msg, 0);

          if (res < 0)
            throw std::runtime_error(fmt::format(
                "Failed to recieve control message. res={}, errno={}", res,
                strerror(errno)));

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
        boost::asio::buffer(message->body.data(), message->header.bodylen),
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

  auto writeControlAsync(std::shared_ptr<Message<MessageType>> message)
      -> void {
    _context.post(_writeStrand.wrap([=]() {
      char data = 'x';
      iovec io = {.iov_base = &data, .iov_len = 1};

      msghdr msg = {0};
      msg.msg_iov = &io;
      msg.msg_iovlen = 1;
      msg.msg_control = message->control.data();
      msg.msg_controllen = message->header.controllen;

      auto res = sendmsg(_socket->native_handle(), &msg, 0);

      if (res < 0)
        throw std::runtime_error("Failed to send control message");
    }));
  }
};

} // namespace gnf
