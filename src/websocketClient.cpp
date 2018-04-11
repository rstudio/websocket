#include <Rcpp.h>
#define ASIO_STANDALONE
#include <iostream>
#include <websocketpp/common/functional.hpp>
#include "client.hpp"

typedef websocketpp::lib::shared_ptr<asio::ssl::context> context_ptr;

using namespace Rcpp;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

static context_ptr on_tls_init() {
  context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
  try {
    ctx->set_options(asio::ssl::context::default_workarounds |
      asio::ssl::context::no_sslv2 |
      asio::ssl::context::no_sslv3 |
      asio::ssl::context::single_dh_use);
  } catch (std::exception &e) {
    std::cout << "Error in context pointer: " << e.what() << std::endl;
  }
  return ctx;
}

class WSConnection {
public:
  WSConnection(websocketpp::lib::shared_ptr<Client> client) : client(client) {};

  enum STATE { INIT, OPEN, CLOSED, FAILED };
  STATE state = INIT;
  websocketpp::lib::shared_ptr<Client> client;
};

websocketpp::lib::shared_ptr<WSConnection> xptrGetClient(SEXP client_xptr) {
  if (TYPEOF(client_xptr) != EXTPTRSXP) {
    throw Rcpp::exception("Expected external pointer.");
  }
  return *reinterpret_cast<websocketpp::lib::shared_ptr<WSConnection>*>(R_ExternalPtrAddr(client_xptr));
}

void client_deleter(SEXP client_xptr) {
  delete reinterpret_cast<websocketpp::lib::shared_ptr<WSConnection>*>(R_ExternalPtrAddr(client_xptr));
  R_ClearExternalPtr(client_xptr);
}

void handleMessage(Rcpp::Function onMessage, websocketpp::connection_hdl, message_ptr msg) {
  const std::string payload = msg->get_payload();
  onMessage(payload);
}

void handleClose(websocketpp::lib::shared_ptr<WSConnection> wsPtr, Rcpp::Function onClose, websocketpp::connection_hdl) {
  wsPtr->state = WSConnection::STATE::CLOSED;
  onClose();
}

void handleOpen(websocketpp::lib::shared_ptr<WSConnection> wsPtr, Rcpp::Function onOpen, websocketpp::connection_hdl) {
  wsPtr->state = WSConnection::STATE::OPEN;
  onOpen();
}

void handleFail(websocketpp::lib::shared_ptr<WSConnection> wsPtr, Rcpp::Function onFail, websocketpp::connection_hdl) {
  wsPtr->state = WSConnection::STATE::FAILED;
  onFail();
}

// [[Rcpp::export]]
SEXP wsCreate(std::string uri, Rcpp::Function onMessage, Rcpp::Function onOpen, Rcpp::Function onClose, Rcpp::Function onFail) {
  if (uri.size() < 6) {
    throw Rcpp::exception("Invalid websocket URI: too short");
  }

  websocketpp::lib::shared_ptr<WSConnection> wsPtr;

  bool tls = false;
  if (uri.substr(0, 5) == "ws://") {
    websocketpp::lib::shared_ptr<ClientImpl<ws_client>> client = websocketpp::lib::make_shared<ClientImpl<ws_client>>();
    wsPtr = websocketpp::lib::make_shared<WSConnection>(client);
  } else if (uri.substr(0, 6) == "wss://") {
    tls = true;
    websocketpp::lib::shared_ptr<ClientImpl<wss_client>> client = websocketpp::lib::make_shared<ClientImpl<wss_client>>();
    wsPtr = websocketpp::lib::make_shared<WSConnection>(client);
  } else {
    throw Rcpp::exception("Invalid websocket URI: must begin with ws:// or wss://");
  }

  wsPtr->client->set_access_channels(websocketpp::log::alevel::all);
  wsPtr->client->clear_access_channels(websocketpp::log::alevel::frame_payload);
  wsPtr->client->init_asio();
  if (tls) {
    wsPtr->client->set_tls_init_handler(bind(&on_tls_init));
  }
  wsPtr->client->set_open_handler(bind(handleOpen, wsPtr, onOpen, ::_1));
  wsPtr->client->set_message_handler(bind(handleMessage, onMessage, ::_1, ::_2));
  wsPtr->client->set_close_handler(bind(handleClose, wsPtr, onClose, ::_1));
  wsPtr->client->set_fail_handler(bind(handleFail, wsPtr, onFail, ::_1));

  websocketpp::lib::error_code ec;
  wsPtr->client->setup_connection(uri, ec);
  if (ec) {
    // TODO Should we call onFail here?
    stop("Could not create connection because: " + ec.message());
  }

  websocketpp::lib::shared_ptr<WSConnection> *extwsPtr = new websocketpp::lib::shared_ptr<WSConnection>(wsPtr);
  SEXP client_xptr = PROTECT(R_MakeExternalPtr(extwsPtr, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(client_xptr, client_deleter, TRUE);
  UNPROTECT(1);
  return client_xptr;
}

// [[Rcpp::export]]
void wsConnect(SEXP client_xptr) {
  websocketpp::lib::shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->connect();
  // Block until the connection is either open, closed, or the attempt to connect has failed.
  // wsPtr->client->run() would block indefinitely, so we use run_one() instead.
  // We don't need to call restart() here because the underlying io_context is never out of work between invocations.
  while (wsPtr->state == WSConnection::STATE::INIT) {
    wsPtr->client->run_one();
  }
}

// [[Rcpp::export]]
void wsRestart(SEXP client_xptr) {
  websocketpp::lib::shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->get_io_service().restart();
}

// [[Rcpp::export]]
void wsPoll(SEXP client_xptr) {
  websocketpp::lib::shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->poll();
}

// [[Rcpp::export]]
void wsSend(SEXP client_xptr, std::string msg) {
  websocketpp::lib::shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->send(msg, websocketpp::frame::opcode::text);
}

// [[Rcpp::export]]
void wsReset(SEXP client_xptr) {
  websocketpp::lib::shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->reset();
}

// [[Rcpp::export]]
void wsClose(SEXP client_xptr) {
  websocketpp::lib::shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->close(websocketpp::close::status::normal, "closing normally");
}

// [[Rcpp::export]]
bool wsStopped(SEXP client_xptr) {
  websocketpp::lib::shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  return wsPtr->client->stopped();
}

// [[Rcpp::export]]
std::string wsState(SEXP client_xptr) {
  websocketpp::lib::shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  switch(wsPtr->state) {
    case WSConnection::STATE::INIT: return "INIT";
    case WSConnection::STATE::OPEN: return "OPEN";
    case WSConnection::STATE::CLOSED: return "CLOSED";
    case WSConnection::STATE::FAILED: return "FAILED";
  }
}
