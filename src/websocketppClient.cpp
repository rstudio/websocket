#include <Rcpp.h>
#define ASIO_STANDALONE
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef std::shared_ptr<asio::ssl::context> context_ptr;

using namespace Rcpp;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;


    websocketpp::lib::error_code ec;

    c->send(hdl, msg->get_payload(), msg->get_opcode(), ec);
    if (ec) {
        std::cout << "Echo failed because: " << ec.message() << std::endl;
    }
}

static context_ptr on_tls_init() {
    // establishes a SSL connection
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

// [[Rcpp::export]]
void wsppTest(std::string uri) {
  client c;
  try {
    c.set_access_channels(websocketpp::log::alevel::all);
    c.clear_access_channels(websocketpp::log::alevel::frame_payload);

    c.init_asio();
    c.set_tls_init_handler(bind(&on_tls_init));
    c.set_message_handler(bind(&on_message, &c, ::_1, ::_2));

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);

    if (ec) {
      std::cout << "could not create connection because: " << ec.message() << std::endl;
      return;
    }

    c.connect(con);
    c.run();
  } catch (websocketpp::exception const & e) {
    std::cout << e.what() << std::endl;
  }
}
// using easywsclient::WebSocket;
//
// WebSocket* xptrGetWs(SEXP ws_xptr) {
//   if (TYPEOF(ws_xptr) != EXTPTRSXP) {
//     throw Rcpp::exception("Expected external pointer.");
//   }
//   return reinterpret_cast<WebSocket*>(R_ExternalPtrAddr(ws_xptr));
// }
//
// void websocket_deleter(SEXP ws_xptr) {
//   delete xptrGetWs(ws_xptr);
//   R_ClearExternalPtr(ws_xptr);
// }
//
// // [[Rcpp::export]]
// SEXP wsCreate(std::string url) {
//   WebSocket *ws = WebSocket::from_url(url);
//   SEXP ws_xptr = PROTECT(R_MakeExternalPtr(ws, R_NilValue, R_NilValue));
//   R_RegisterCFinalizerEx(ws_xptr, websocket_deleter, TRUE);
//   UNPROTECT(1);
//   return ws_xptr;
// }
//
// // [[Rcpp::export]]
// void wsSend(SEXP ws_xptr, std::string msg) {
//   WebSocket *ws = xptrGetWs(ws_xptr);
//   if (ws->getReadyState() == WebSocket::readyStateValues::CLOSED) {
//     throw Rcpp::exception("Can't send, WebSocket is closed.");
//   }
//   ws->send(msg);
//   ws->poll();
// }
//
// // [[Rcpp::export]]
// void wsReceive(SEXP ws_xptr, Rcpp::Function onMessage) {
//   WebSocket *ws = xptrGetWs(ws_xptr);
//   ws->poll();
//   ws->dispatch(onMessage);
// }
//
// // [[Rcpp::export]]
// void wsClose(SEXP ws_xptr) {
//   WebSocket *ws = xptrGetWs(ws_xptr);
//   if (ws->getReadyState() == WebSocket::readyStateValues::CLOSED) {
//     throw Rcpp::exception("Can't close, WebSocket is already closed.");
//   }
//   ws->close();
//   // Here the state is CLOSING, calling poll() ensures the state will transition to CLOSED before this function returns.
//   ws->poll();
// }
//
// // [[Rcpp::export]]
// std::string wsState(SEXP ws_xptr) {
//   WebSocket *ws = xptrGetWs(ws_xptr);
//   ws->poll();
//   WebSocket::readyStateValues state = ws->getReadyState();
//   switch(state) {
//     case WebSocket::readyStateValues::CLOSED: return "CLOSED";
//     case WebSocket::readyStateValues::CLOSING: return "CLOSING";
//     case WebSocket::readyStateValues::CONNECTING: return "CONNECTING";
//     case WebSocket::readyStateValues::OPEN: return "OPEN";
//   }
// }
