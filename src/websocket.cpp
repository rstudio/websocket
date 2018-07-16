#include <Rcpp.h>

#ifdef _WIN32
// Taken from http://tolstoy.newcastle.edu.au/R/e2/devel/06/11/1242.html
// Undefine the Realloc macro, which is defined by both R and by Windows stuff
#undef Realloc
// Also need to undefine the Free macro
#undef Free

#include <winsock2.h>
#include <windows.h>

// These are defined by windows.h but we have to undefine them so that the
// typedef enum Rboolean will be used later on.
#undef TRUE
#undef FALSE

#endif // _WIN32


#define ASIO_STANDALONE
#include <iostream>
#include <websocketpp/common/functional.hpp>
#include "client.hpp"


using namespace Rcpp;

// The websocketpp/common/functional.hpp file detects if a C++11 compiler is
// used. If so, ws_websocketpp::lib::shared_ptr is a std::shared_ptr. If not,
// ws_websocketpp::lib::shared_ptr is a boost::shared_ptr.
using ws_websocketpp::lib::shared_ptr;
using ws_websocketpp::lib::make_shared;

using ws_websocketpp::lib::placeholders::_1;
using ws_websocketpp::lib::placeholders::_2;
using ws_websocketpp::lib::bind;

typedef shared_ptr<asio::ssl::context> context_ptr;


static context_ptr on_tls_init() {
  context_ptr ctx = make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
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
  WSConnection(shared_ptr<Client> client) : client(client) {};

  enum STATE { INIT, OPEN, CLOSED, FAILED };
  STATE state = INIT;
  shared_ptr<Client> client;
};


shared_ptr<WSConnection> xptrGetClient(SEXP client_xptr) {
  if (TYPEOF(client_xptr) != EXTPTRSXP) {
    throw Rcpp::exception("Expected external pointer.");
  }
  return *reinterpret_cast<shared_ptr<WSConnection>*>(R_ExternalPtrAddr(client_xptr));
}

void client_deleter(SEXP client_xptr) {
  delete reinterpret_cast<shared_ptr<WSConnection>*>(R_ExternalPtrAddr(client_xptr));
  R_ClearExternalPtr(client_xptr);
}

void handleMessage(Rcpp::Function onMessage, ws_websocketpp::connection_hdl, message_ptr msg) {
  ws_websocketpp::frame::opcode::value opcode = msg->get_opcode();
  if (opcode == ws_websocketpp::frame::opcode::value::text) {
    onMessage(msg->get_payload());

  } else if (opcode == ws_websocketpp::frame::opcode::value::binary) {
    const std::string msg_str = msg->get_payload();
    onMessage(std::vector<uint8_t>(msg_str.begin(), msg_str.end()));

  } else {
    stop("Unknown opcode for message (not text or binary).");
  }
}

void handleClose(shared_ptr<WSConnection> wsPtr, Rcpp::Function onClose, ws_websocketpp::connection_hdl) {
  wsPtr->state = WSConnection::STATE::CLOSED;
  onClose();
}

void handleOpen(shared_ptr<WSConnection> wsPtr, Rcpp::Function onOpen, ws_websocketpp::connection_hdl) {
  wsPtr->state = WSConnection::STATE::OPEN;
  onOpen();
}

void handleFail(shared_ptr<WSConnection> wsPtr, Rcpp::Function onFail, ws_websocketpp::connection_hdl) {
  wsPtr->state = WSConnection::STATE::FAILED;
  onFail();
}

// [[Rcpp::export]]
SEXP wsCreate(
  std::string uri,
  Rcpp::Function onMessage,
  Rcpp::Function onOpen,
  Rcpp::Function onClose,
  Rcpp::Function onFail,
  Rcpp::CharacterVector accessLogChannels,
  Rcpp::CharacterVector errorLogChannels
) {
  if (uri.size() < 6) {
    throw Rcpp::exception("Invalid websocket URI: too short");
  }

  shared_ptr<WSConnection> wsPtr;

  if (uri.substr(0, 5) == "ws://") {
    shared_ptr<ClientImpl<ws_client>> client = make_shared<ClientImpl<ws_client>>();
    wsPtr = make_shared<WSConnection>(client);

  } else if (uri.substr(0, 6) == "wss://") {
    shared_ptr<ClientImpl<wss_client>> client = make_shared<ClientImpl<wss_client>>();
    wsPtr = make_shared<WSConnection>(client);
    wsPtr->client->set_tls_init_handler(bind(&on_tls_init));

  } else {
    throw Rcpp::exception("Invalid websocket URI: must begin with ws:// or wss://");
  }

  wsPtr->client->update_log_channels("access", "set", accessLogChannels);
  wsPtr->client->update_log_channels("error", "set", errorLogChannels);
  wsPtr->client->init_asio();
  wsPtr->client->set_open_handler(bind(handleOpen, wsPtr, onOpen, ::_1));
  wsPtr->client->set_message_handler(bind(handleMessage, onMessage, ::_1, ::_2));
  wsPtr->client->set_close_handler(bind(handleClose, wsPtr, onClose, ::_1));
  wsPtr->client->set_fail_handler(bind(handleFail, wsPtr, onFail, ::_1));

  ws_websocketpp::lib::error_code ec;
  wsPtr->client->setup_connection(uri, ec);
  if (ec) {
    // TODO Should we call onFail here?
    stop("Could not create connection because: " + ec.message());
  }

  shared_ptr<WSConnection> *extwsPtr = new shared_ptr<WSConnection>(wsPtr);
  SEXP client_xptr = PROTECT(R_MakeExternalPtr(extwsPtr, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(client_xptr, client_deleter, TRUE);
  UNPROTECT(1);
  return client_xptr;
}

// [[Rcpp::export]]
void wsAppendHeader(SEXP client_xptr, std::string key, std::string value) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->append_header(key, value);
}

// [[Rcpp::export]]
void wsConnect(SEXP client_xptr) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
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
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->get_io_service().restart();
}

// [[Rcpp::export]]
void wsPoll(SEXP client_xptr) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->poll();
}

// [[Rcpp::export]]
void wsSend(SEXP client_xptr, SEXP msg) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);

  if (TYPEOF(msg) == STRSXP &&
      Rf_length(msg) == 1 &&
      STRING_ELT(msg, 0) != NA_STRING)
  {
    const char* msg_ptr = CHAR(STRING_ELT(msg, 0));
    int len = R_nchar(STRING_ELT(msg, 0), Bytes, FALSE, FALSE, "wsSend");
    wsPtr->client->send(msg_ptr, len, ws_websocketpp::frame::opcode::text);

  } else if (TYPEOF(msg) == RAWSXP) {
    wsPtr->client->send(RAW(msg), Rf_length(msg), ws_websocketpp::frame::opcode::binary);

  } else {
    stop("msg must be a one-element character vector or a raw vector.");
  }
}

// [[Rcpp::export]]
void wsReset(SEXP client_xptr) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->reset();
}

// [[Rcpp::export]]
void wsClose(SEXP client_xptr) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->close(ws_websocketpp::close::status::normal, "closing normally");
}

// [[Rcpp::export]]
bool wsStopped(SEXP client_xptr) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  return wsPtr->client->stopped();
}

// [[Rcpp::export]]
std::string wsState(SEXP client_xptr) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  switch(wsPtr->state) {
    case WSConnection::STATE::INIT: return "INIT";
    case WSConnection::STATE::OPEN: return "OPEN";
    case WSConnection::STATE::CLOSED: return "CLOSED";
    case WSConnection::STATE::FAILED: return "FAILED";
  }

  // Shouldn't be possible to get here, but some compilers still complain
  // about reaching end of a non-void function.
  return "UNKNOWN";
}




// [[Rcpp::export]]
void wsUpdateLogChannels(
  SEXP client_xptr,
  std::string accessOrError,
  std::string setOrClear,
  Rcpp::CharacterVector logChannels
) {
  shared_ptr<WSConnection> wsPtr = xptrGetClient(client_xptr);
  wsPtr->client->update_log_channels(accessOrError, setOrClear, logChannels);
}
