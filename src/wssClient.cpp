#include <Rcpp.h>
#define ASIO_STANDALONE
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

typedef std::shared_ptr<asio::ssl::context> context_ptr;
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef websocketpp::lib::function<void(websocketpp::connection_hdl, message_ptr)> message_handler;
typedef websocketpp::lib::function<void(websocketpp::connection_hdl)> close_handler;

using namespace Rcpp;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

static context_ptr on_tls_init() {
  context_ptr ctx = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
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

struct WSSConnection {
  enum STATE { INIT, OPEN, CLOSED, FAILED };
  STATE state = INIT;
  client client;
  client::connection_ptr con;
};

boost::shared_ptr<WSSConnection> xptrGetClient(SEXP client_xptr) {
  if (TYPEOF(client_xptr) != EXTPTRSXP) {
    throw Rcpp::exception("Expected external pointer.");
  }
  return *reinterpret_cast<boost::shared_ptr<WSSConnection>*>(R_ExternalPtrAddr(client_xptr));
}

void client_deleter(SEXP client_xptr) {
  delete reinterpret_cast<boost::shared_ptr<WSSConnection>*>(R_ExternalPtrAddr(client_xptr));
  R_ClearExternalPtr(client_xptr);
}

void handleMessage(Rcpp::Function onMessage, websocketpp::connection_hdl, message_ptr msg) {
  const std::string payload = msg->get_payload();
  onMessage(payload);
}

void handleClose(boost::shared_ptr<WSSConnection> wssPtr, Rcpp::Function onClose, websocketpp::connection_hdl) {
  wssPtr->state = WSSConnection::STATE::CLOSED;
  onClose();
}

void handleOpen(boost::shared_ptr<WSSConnection> wssPtr, Rcpp::Function onOpen, websocketpp::connection_hdl) {
  wssPtr->state = WSSConnection::STATE::OPEN;
  onOpen();
}

void handleFail(boost::shared_ptr<WSSConnection> wssPtr, Rcpp::Function onFail, websocketpp::connection_hdl) {
  wssPtr->state = WSSConnection::STATE::FAILED;
  onFail();
}

// [[Rcpp::export]]
SEXP wssCreate(std::string uri, Rcpp::Function onMessage, Rcpp::Function onOpen, Rcpp::Function onClose, Rcpp::Function onFail) {
  boost::shared_ptr<WSSConnection> wssPtr = boost::make_shared<WSSConnection>();

  wssPtr->client.set_access_channels(websocketpp::log::alevel::all);
  wssPtr->client.clear_access_channels(websocketpp::log::alevel::frame_payload);
  wssPtr->client.init_asio();
  wssPtr->client.set_tls_init_handler(bind(&on_tls_init));
  wssPtr->client.set_open_handler(bind(handleOpen, wssPtr, onOpen, ::_1));
  wssPtr->client.set_message_handler(bind(handleMessage, onMessage, ::_1, ::_2));
  wssPtr->client.set_close_handler(bind(handleClose, wssPtr, onClose, ::_1));
  wssPtr->client.set_fail_handler(bind(handleFail, wssPtr, onFail, ::_1));

  websocketpp::lib::error_code ec;
  wssPtr->con = wssPtr->client.get_connection(uri, ec);
  if (ec) {
    // TODO Should we call onFail here?
    stop("Could not create connection because: " + ec.message());
  }

  boost::shared_ptr<WSSConnection> *extWssPtr = new boost::shared_ptr<WSSConnection>(wssPtr);
  SEXP client_xptr = PROTECT(R_MakeExternalPtr(extWssPtr, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(client_xptr, client_deleter, TRUE);
  UNPROTECT(1);
  return client_xptr;
}

// [[Rcpp::export]]
void wssConnect(SEXP client_xptr) {
  boost::shared_ptr<WSSConnection> wssPtr = xptrGetClient(client_xptr);
  wssPtr->client.connect(wssPtr->con);
  // Block until the connection is either open, closed, or the attempt to connect has failed.
  // wssPtr->client.run() would block indefinitely, so we use run_one() instead.
  // We don't need to call restart() here because the underlying io_context is never out of work between invocations.
  while (wssPtr->state == WSSConnection::STATE::INIT) {
    wssPtr->client.run_one();
  }
}

// [[Rcpp::export]]
void wssRestart(SEXP client_xptr) {
  boost::shared_ptr<WSSConnection> wssPtr = xptrGetClient(client_xptr);
  wssPtr->client.get_io_service().restart();
}

// [[Rcpp::export]]
void wssPoll(SEXP client_xptr) {
  boost::shared_ptr<WSSConnection> wssPtr = xptrGetClient(client_xptr);
  wssPtr->client.poll();
}

// [[Rcpp::export]]
void wssSend(SEXP client_xptr, std::string msg) {
  boost::shared_ptr<WSSConnection> wssPtr = xptrGetClient(client_xptr);
  wssPtr->client.send(wssPtr->con, msg, websocketpp::frame::opcode::text);
}

// [[Rcpp::export]]
void wssReset(SEXP client_xptr) {
  boost::shared_ptr<WSSConnection> wssPtr = xptrGetClient(client_xptr);
  wssPtr->client.reset();
}

// [[Rcpp::export]]
void wssClose(SEXP client_xptr) {
  boost::shared_ptr<WSSConnection> wssPtr = xptrGetClient(client_xptr);
  wssPtr->client.close(wssPtr->con, websocketpp::close::status::normal, "closing normally");
}

// [[Rcpp::export]]
bool wssStopped(SEXP client_xptr) {
  boost::shared_ptr<WSSConnection> wssPtr = xptrGetClient(client_xptr);
  return wssPtr->client.stopped();
}

// [[Rcpp::export]]
std::string wssState(SEXP client_xptr) {
  boost::shared_ptr<WSSConnection> wssPtr = xptrGetClient(client_xptr);
  switch(wssPtr->state) {
    case WSSConnection::STATE::INIT: return "INIT";
    case WSSConnection::STATE::OPEN: return "OPEN";
    case WSSConnection::STATE::CLOSED: return "CLOSED";
    case WSSConnection::STATE::FAILED: return "FAILED";
  }
}
