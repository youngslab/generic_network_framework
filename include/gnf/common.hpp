#pragma once

#include <boost/asio.hpp>

namespace gnf {

template <typename Protocol> struct get_acceptor;

template <> struct get_acceptor<boost::asio::ip::tcp> {
  using type = boost::asio::ip::tcp::acceptor;
};

template <> struct get_acceptor<boost::asio::local::stream_protocol> {
  using type = boost::asio::local::stream_protocol::acceptor;
};

template <typename Protocol>
using get_acceptor_t = typename get_acceptor<Protocol>::type;

template <typename Protocol> struct get_endpoint;

template <> struct get_endpoint<boost::asio::local::stream_protocol> {
  using type = boost::asio::local::stream_protocol::endpoint;
};

template <> struct get_endpoint<boost::asio::ip::tcp> {
  using type = boost::asio::ip::tcp::endpoint;
};

template <typename Protocol>
using get_endpoint_t = typename get_endpoint<Protocol>::type;

template <typename Protocol> struct get_socket;

template <> struct get_socket<boost::asio::local::stream_protocol> {
  using type = boost::asio::local::stream_protocol::socket;
};

template <> struct get_socket<boost::asio::ip::tcp> {
  using type = boost::asio::ip::tcp::socket;
};

template <typename Protocol>
using get_socket_t = typename get_socket<Protocol>::type;

} // namespace gnf
