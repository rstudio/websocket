#ifndef WEBSOCKET_DEFS_HPP
#define WEBSOCKET_DEFS_HPP

#include "client.hpp"
#include "wrapped_print.h"
#include <websocketpp/common/functional.hpp>

// The websocketpp/common/functional.hpp file detects if a C++11 compiler is
// used. If so, ws_websocketpp::lib::shared_ptr is a std::shared_ptr. If not,
// ws_websocketpp::lib::shared_ptr is a boost::shared_ptr.
using ws_websocketpp::lib::shared_ptr;
using ws_websocketpp::lib::weak_ptr;
using ws_websocketpp::lib::make_shared;
using ws_websocketpp::lib::enable_shared_from_this;

using ws_websocketpp::lib::placeholders::_1;
using ws_websocketpp::lib::placeholders::_2;
using ws_websocketpp::lib::bind;

typedef shared_ptr<asio::ssl::context> context_ptr;

#endif
