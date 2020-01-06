#include "websocket_task.h"

// ============================================================================
// Websocket thread
// ============================================================================

#include <Rcpp.h>
#include "websocket_defs.h"
#include "websocket_task.h"

using ws_websocketpp::lib::function;
using ws_websocketpp::lib::bind;


static context_ptr on_tls_init() {
  context_ptr ctx = make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
  try {
    ctx->set_options(asio::ssl::context::default_workarounds |
      asio::ssl::context::no_sslv2 |
      asio::ssl::context::no_sslv3 |
      asio::ssl::context::single_dh_use);
  } catch (std::exception &e) {
    Rcpp::Rcout << "Error in context pointer: " << e.what() << std::endl;
  }
  return ctx;
}

// Invoke a callback and delete the object. The Callback object must have been
// heap-allocated.
void invoke_function_callback(void* data) {
  function<void (void)>* fun = reinterpret_cast<function<void (void)>*>(data);
  (*fun)();
  delete fun;
}


WebsocketTask::WebsocketTask(
  std::string uri,
  Rcpp::Environment robjPublic,
  Rcpp::Environment robjPrivate,
  Rcpp::CharacterVector accessLogChannels,
  Rcpp::CharacterVector errorLogChannels,
  int maxMessageSize)
: uri(uri),
  robjPublic(robjPublic),
  robjPrivate(robjPrivate),
  accessLogChannels(accessLogChannels),
  errorLogChannels(errorLogChannels)
{
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
  client->set_open_handler(   bind(&WebsocketTask::handleOpen,    this, ::_1));
  client->set_message_handler(bind(&WebsocketTask::handleMessage, this, ::_1, ::_2));
  client->set_close_handler(  bind(&WebsocketTask::handleClose,   this, ::_1));
  client->set_fail_handler(   bind(&WebsocketTask::handleFail,    this, ::_1));

  client->set_max_message_size(maxMessageSize);

  ws_websocketpp::lib::error_code ec;
  client->setup_connection(uri, ec);
  if (ec) {
    // TODO Should we call onFail here?
    Rcpp::stop("Could not create connection because: " + ec.message());
  }
}

void WebsocketTask::handleMessage(ws_websocketpp::connection_hdl, message_ptr msg) {
  // TODO: Add thread assertions
  later::later(
    invoke_function_callback,
    new function<void (void)>(bind(&WebsocketTask::rHandleMessage, this, msg)),
    0
  );
}

void WebsocketTask::rHandleMessage(message_ptr msg) {
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

void WebsocketTask::handleClose(ws_websocketpp::connection_hdl) {
  ws_websocketpp::close::status::value code = client->get_remote_close_code();
  std::string reason = client->get_remote_close_reason();

  later::later(
    invoke_function_callback,
    new function<void (void)>(bind(&WebsocketTask::rHandleClose, this, code, reason)),
    0
  );
}

void WebsocketTask::rHandleClose(ws_websocketpp::close::status::value code, std::string reason) {
  // Rcpp::Rcerr << "WebsocketTask::rHandleClose\n";

  state = WebsocketTask::STATE::CLOSED;
  Rcpp::List event;
  event["target"] = robjPublic;
  event["code"] = code;
  event["reason"] = reason;

  Rcpp::Function onClose = getInvoker("close");
  removeHandlers();
  onClose(event);
}


void WebsocketTask::handleOpen(ws_websocketpp::connection_hdl) {
  // std::cerr << "WebsocketTask::handleOpen\n";
  if (closeOnOpen) {
    state = WebsocketTask::STATE::CLOSING;
    client->close(ws_websocketpp::close::status::normal, "");
    return;
  }

  state = WebsocketTask::STATE::OPEN;

  later::later(
    invoke_function_callback,
    new function<void (void)>(bind(&WebsocketTask::rHandleOpen, this)),
    0
  );
}

void WebsocketTask::rHandleOpen() {
  Rcpp::List event;
  event["target"] = robjPublic;
  getInvoker("open")(event);
}


void WebsocketTask::handleFail(ws_websocketpp::connection_hdl) {
  state = WebsocketTask::STATE::FAILED;

  later::later(
    invoke_function_callback,
    new function<void (void)>(bind(&WebsocketTask::rHandleFail, this)),
    0
  );
}

void WebsocketTask::rHandleFail() {
  ws_websocketpp::lib::error_code ec = client->get_ec();
  std::string errMessage = ec.message();

  Rcpp::List event;
  event["target"] = robjPublic;
  event["message"] = errMessage;

  Rcpp::Function onFail = getInvoker("error");
  removeHandlers();
  onFail(event);
}



void WebsocketTask::connect() {
  // std::cerr << "WebsocketTask::connect\n";
  // Starts a new thread in which execute is called.
  this->begin();
}

void WebsocketTask::close(uint16_t code, std::string reason) {
  switch (state) {
  case WebsocketTask::STATE::INIT:
    closeOnOpen = true;
    return;
  case WebsocketTask::STATE::OPEN:
    break;
  case WebsocketTask::STATE::CLOSING:
  case WebsocketTask::STATE::CLOSED:
  case WebsocketTask::STATE::FAILED:
    return;
  }

  state = WebsocketTask::STATE::CLOSING;
  client->close(code, reason);
}

void WebsocketTask::removeHandlers() {
  robjPublic = Rcpp::Environment();
  robjPrivate = Rcpp::Environment();
}

void WebsocketTask::execute() {
  // std::cerr << "WebsocketTask::execute\n";
  client->connect();
  client->run();
}

void WebsocketTask::complete() {
  // std::cerr << "WebsocketTask::complete\n";
}

Rcpp::Function WebsocketTask::getInvoker(std::string name) {
  Rcpp::Function gi = robjPrivate.get("getInvoker");
  return gi(name);
}
