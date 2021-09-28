
#pragma once

#include <boost/asio.hpp>

static const std::string SOCKET_PATH = "/home/jaeyoungs/chat_socket";

static const int PORT = 8876;

static const char *IP = "127.0.0.1";

enum class ChatMessageType { Message, FD };

