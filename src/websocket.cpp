#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>

// These are defined by windows.h but we have to undefine them so that the
// typedef enum Rboolean will be used later on.
#undef TRUE
#undef FALSE

#endif // _WIN32


#include <iostream>
#include "cpp11.hpp"
#include "wrapped_print.h"
#include <websocketpp/common/functional.hpp>
#include "client.hpp"
#include "websocket_defs.h"
#include "websocket_task.h"
#include "websocket_connection.h"
#include "debug.h"


shared_ptr<WebsocketConnection> xptrGetWsConn(SEXP wsc_xptr) {
  if (TYPEOF(wsc_xptr) != EXTPTRSXP) {
    cpp11::stop("Expected external pointer.");
  }
  return *reinterpret_cast<shared_ptr<WebsocketConnection>*>(R_ExternalPtrAddr(wsc_xptr));
}

void wsc_deleter(SEXP wsc_xptr) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);
  if (!wsc->client->stopped()) {
    // I don't think we'll ever actually get here because as long as the
    // connection is open, the WebsocketConnection object will contain a
    // reference to the R6 object (robjPublic), which contains a reference
    // back to the WebsocketConnection object. This prevents the R6 object
    // from getting GC'd, and its external pointer in turn will not get GC'd.
    // But just in case we get here somehow, we'll stop the thread.
    wsc->client->stop();
  }
  delete reinterpret_cast<shared_ptr<WebsocketConnection>*>(R_ExternalPtrAddr(wsc_xptr));
  R_ClearExternalPtr(wsc_xptr);
}


[[cpp11::register]]
SEXP wsCreate(
  std::string uri,
  int loop_id,
  cpp11::environment robjPublic,
  cpp11::environment robjPrivate,
  cpp11::strings accessLogChannels,
  cpp11::strings errorLogChannels,
  int maxMessageSize
) {
  REGISTER_MAIN_THREAD()
  WebsocketConnection* wsc = new WebsocketConnection(
    uri, loop_id, robjPublic, robjPrivate, accessLogChannels, errorLogChannels, maxMessageSize
  );

  shared_ptr<WebsocketConnection> *wsc_pp = new shared_ptr<WebsocketConnection>(wsc);
  SEXP wsc_xptr = PROTECT(R_MakeExternalPtr(wsc_pp, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(wsc_xptr, wsc_deleter, TRUE);
  UNPROTECT(1);
  return wsc_xptr;
}

[[cpp11::register]]
void wsAppendHeader(SEXP wsc_xptr, std::string key, std::string value) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);
  wsc->client->append_header(key, value);
}

[[cpp11::register]]
void wsAddProtocols(SEXP wsc_xptr, cpp11::strings protocols) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);
  for (auto it = protocols.begin(); it != protocols.end(); ++it) {
    std::string protocol = *it;
    wsc->client->add_subprotocol(protocol);
  }
}

[[cpp11::register]]
void wsAddProxy(SEXP wsc_xptr, std::string proxy_url) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);
  wsc->client->set_proxy(proxy_url);
}

[[cpp11::register]]
void wsConnect(SEXP wsc_xptr) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);

  wsc->client->connect();

  // Starts a new thread in which WebsocketTask::execute is called. Object is
  // automatically deleted when thread stops.
  WebsocketTask* wst = new WebsocketTask(wsc);
  wst->begin();
}

[[cpp11::register]]
void wsSend(SEXP wsc_xptr, SEXP msg) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);

  if (TYPEOF(msg) == STRSXP &&
      Rf_length(msg) == 1 &&
      STRING_ELT(msg, 0) != NA_STRING)
  {
    // TODO: Make sure that message lifetime is long enough
    const char* msg_ptr = CHAR(STRING_ELT(msg, 0));
    int len = R_nchar(STRING_ELT(msg, 0), Bytes, FALSE, FALSE, "wsSend");
    // When send() is called, I believe that the message is immediately
    // appended to the message buffer, and then the actual sending of the
    // message is queued. Since a copy of the message is made, it should be
    // safe if the original msg is cleared before the actual send happens.
    wsc->client->send(msg_ptr, len, ws_websocketpp::frame::opcode::text);

  } else if (TYPEOF(msg) == RAWSXP) {
    wsc->client->send(RAW(msg), Rf_length(msg), ws_websocketpp::frame::opcode::binary);
  } else {
    cpp11::stop("msg must be a one-element character vector or a raw vector.");
  }
}

[[cpp11::register]]
void wsClose(SEXP wsc_xptr, uint16_t code, std::string reason) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);
  wsc->close(code, reason);
}

[[cpp11::register]]
std::string wsProtocol(SEXP wsc_xptr) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);
  return wsc->client->get_subprotocol();
}

[[cpp11::register]]
std::string wsState(SEXP wsc_xptr) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);
  switch(wsc->state) {
    case WebsocketConnection::STATE::INIT: return "INIT";
    case WebsocketConnection::STATE::OPEN: return "OPEN";
    case WebsocketConnection::STATE::CLOSING: return "CLOSING";
    case WebsocketConnection::STATE::CLOSED: return "CLOSED";
    case WebsocketConnection::STATE::FAILED: return "FAILED";
  }

  // Shouldn't be possible to get here, but some compilers still complain
  // about reaching end of a non-void function.
  return "UNKNOWN";
}

[[cpp11::register]]
void wsUpdateLogChannels(
  SEXP wsc_xptr,
  std::string accessOrError,
  std::string setOrClear,
  cpp11::strings logChannels
) {
  ASSERT_MAIN_THREAD()
  shared_ptr<WebsocketConnection> wsc = xptrGetWsConn(wsc_xptr);
  wsc->client->update_log_channels(accessOrError, setOrClear, logChannels);
}
