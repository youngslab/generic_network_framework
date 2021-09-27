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

template <typename Protocol> struct is_tcp : public std::false_type {};

template <> struct is_tcp<boost::asio::ip::tcp> : public std::true_type {};

template <typename T> static constexpr bool is_tcp_v = is_tcp<T>::value_type;

template <typename Protocol>
struct is_stream_protocol : public std::false_type {};

template <>
struct is_stream_protocol<boost::asio::local::stream_protocol>
    : public std::true_type {};

template <typename T>
static constexpr bool is_stream_protocol_v = is_stream_protocol<T>::value_type;
} // namespace gnf
