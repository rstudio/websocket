#include <Rcpp.h>
#include "websocket_defs.h"
#include "websocket_connection.h"
#include "debug.h"

using ws_websocketpp::lib::function;
using ws_websocketpp::lib::bind;


static context_ptr on_tls_init() {
  ASSERT_MAIN_THREAD()
  context_ptr ctx = make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
  try {
    ctx->set_options(asio::ssl::context::default_workarounds |
      asio::ssl::context::no_sslv2 |
      asio::ssl::context::no_sslv3 |
      asio::ssl::context::single_dh_use);
  } catch (std::exception &e) {
    Rcpp::Rcerr << "Error in context pointer: " << e.what() << std::endl;
  }
  return ctx;
}

// Invoke a callback and delete the object. The Callback object must have been
// heap-allocated.
void invoke_function_callback(void* data) {
  ASSERT_MAIN_THREAD()
  function<void (void)>* fun = reinterpret_cast<function<void (void)>*>(data);
  (*fun)();
  delete fun;
}


WebsocketConnection::WebsocketConnection(
  std::string uri,
  int loop_id,
  Rcpp::Environment robjPublic,
  Rcpp::Environment robjPrivate,
  Rcpp::CharacterVector accessLogChannels,
  Rcpp::CharacterVector errorLogChannels,
  int maxMessageSize
)
: uri(uri),
  loop_id(loop_id),
  robjPublic(robjPublic),
  robjPrivate(robjPrivate)
{
  ASSERT_MAIN_THREAD()
  if (uri.size() < 6) {
    throw Rcpp::exception("Invalid websocket URI: too short");
  }


  if (uri.substr(0, 5) == "ws://") {
    client = make_shared<ClientImpl<ws_client>>();

  } else if (uri.substr(0, 6) == "wss://") {
    client = make_shared<ClientImpl<wss_client>>();
    client->set_tls_init_handler(bind(&on_tls_init));

  } else {
    throw Rcpp::exception("Invalid websocket URI: must begin with ws:// or wss://");
  }

  if (accessLogChannels.size() > 0) {
    // clear all channels and set user channels
    client->clear_access_channels(ws_websocketpp::log::alevel::all);
    client->update_log_channels("access", "set", accessLogChannels);
  }
  if (errorLogChannels.size() > 0) {
    // clear all channels and set user channels
    client->clear_error_channels(ws_websocketpp::log::elevel::all);
    client->update_log_channels("error", "set", errorLogChannels);
  }
  client->init_asio();
  client->set_open_handler(   bind(&WebsocketConnection::handleOpen,    this, ::_1));
  client->set_message_handler(bind(&WebsocketConnection::handleMessage, this, ::_1, ::_2));
  client->set_close_handler(  bind(&WebsocketConnection::handleClose,   this, ::_1));
  client->set_fail_handler(   bind(&WebsocketConnection::handleFail,    this, ::_1));

  client->set_max_message_size(maxMessageSize);

  ws_websocketpp::lib::error_code ec;
  client->setup_connection(uri, ec);
  if (ec) {
    // TODO Should we call onFail here?
    Rcpp::stop("Could not create connection because: " + ec.message());
  }
}

void WebsocketConnection::handleMessage(ws_websocketpp::connection_hdl, message_ptr msg) {
  ASSERT_BACKGROUND_THREAD()
  // Note that message_ptr is a shared_ptr, so the lifetime of msg will
  // continue until it's used by rHandleMessage().
  later::later(
    invoke_function_callback,
    new function<void (void)>(bind(&WebsocketConnection::rHandleMessage, this, msg)),
    0,
    loop_id
  );
}

void WebsocketConnection::rHandleMessage(message_ptr msg) {
  ASSERT_MAIN_THREAD()
  Rcpp::List event;
  event["target"] = robjPublic;

  ws_websocketpp::frame::opcode::value opcode = msg->get_opcode();
  if (opcode == ws_websocketpp::frame::opcode::value::text) {
    event["data"] = msg->get_payload();

  } else if (opcode == ws_websocketpp::frame::opcode::value::binary) {
    const std::string msg_str = msg->get_payload();
    event["data"] = std::vector<uint8_t>(msg_str.begin(), msg_str.end());

  } else {
    Rcpp::stop("Unknown opcode for message (not text or binary).");
  }

  getInvoker("message")(event);
}

void WebsocketConnection::handleClose(ws_websocketpp::connection_hdl) {
  ASSERT_BACKGROUND_THREAD()
  ws_websocketpp::close::status::value code = client->get_remote_close_code();
  std::string reason = client->get_remote_close_reason();

  later::later(
    invoke_function_callback,
    new function<void (void)>(bind(&WebsocketConnection::rHandleClose, this, code, reason)),
    0,
    loop_id
  );
}

void WebsocketConnection::rHandleClose(ws_websocketpp::close::status::value code, std::string reason) {
  ASSERT_MAIN_THREAD()
  state = WebsocketConnection::STATE::CLOSED;
  Rcpp::List event;
  event["target"] = robjPublic;
  event["code"] = code;
  event["reason"] = reason;

  Rcpp::Function onClose = getInvoker("close");
  removeHandlers();
  onClose(event);
}


void WebsocketConnection::handleOpen(ws_websocketpp::connection_hdl) {
  ASSERT_BACKGROUND_THREAD()
  later::later(
    invoke_function_callback,
    new function<void (void)>(bind(&WebsocketConnection::rHandleOpen, this)),
    0,
    loop_id
  );
}

void WebsocketConnection::rHandleOpen() {
  ASSERT_MAIN_THREAD()
  if (closeOnOpen) {
    state = WebsocketConnection::STATE::CLOSING;
    client->close(ws_websocketpp::close::status::normal, "");
    return;
  }
  state = WebsocketConnection::STATE::OPEN;

  Rcpp::List event;
  event["target"] = robjPublic;
  getInvoker("open")(event);
}


void WebsocketConnection::handleFail(ws_websocketpp::connection_hdl) {
  ASSERT_BACKGROUND_THREAD()
  later::later(
    invoke_function_callback,
    new function<void (void)>(bind(&WebsocketConnection::rHandleFail, this)),
    0,
    loop_id
  );
}

void WebsocketConnection::rHandleFail() {
  ASSERT_MAIN_THREAD()
  state = WebsocketConnection::STATE::FAILED;

  ws_websocketpp::lib::error_code ec = client->get_ec();
  std::string errMessage = ec.message();

  Rcpp::List event;
  event["target"] = robjPublic;
  event["message"] = errMessage;

  Rcpp::Function onFail = getInvoker("error");
  removeHandlers();
  onFail(event);
}

void WebsocketConnection::close(uint16_t code, std::string reason) {
  ASSERT_MAIN_THREAD()
  switch (state) {
  case WebsocketConnection::STATE::INIT:
    closeOnOpen = true;
    return;
  case WebsocketConnection::STATE::OPEN:
    break;
  case WebsocketConnection::STATE::CLOSING:
  case WebsocketConnection::STATE::CLOSED:
  case WebsocketConnection::STATE::FAILED:
    return;
  }

  state = WebsocketConnection::STATE::CLOSING;
  client->close(code, reason);
}

void WebsocketConnection::removeHandlers() {
  ASSERT_MAIN_THREAD()
  // Clear the references to the parts of the WebSocket R6 object. This is
  // necessary for the WebSocket R6 object to get GC'd by R.
  robjPublic = Rcpp::Environment();
  robjPrivate = Rcpp::Environment();
}

Rcpp::Function WebsocketConnection::getInvoker(std::string name) {
  ASSERT_MAIN_THREAD()
  Rcpp::Function gi = robjPrivate.get("getInvoker");
  return gi(name);
}
