#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>

// These are defined by windows.h but we have to undefine them so that the
// typedef enum Rboolean will be used later on.
#undef TRUE
#undef FALSE

#endif // _WIN32


#include <iostream>
#include <Rcpp.h>
#include <websocketpp/common/functional.hpp>
#include "client.hpp"
#include "websocket_defs.h"
#include "websocket_task.h"

using namespace Rcpp;


shared_ptr<WebsocketTask> xptrGetWsTask(SEXP wstask_xptr) {
  if (TYPEOF(wstask_xptr) != EXTPTRSXP) {
    throw Rcpp::exception("Expected external pointer.");
  }
  return *reinterpret_cast<shared_ptr<WebsocketTask>*>(R_ExternalPtrAddr(wstask_xptr));
}

// TODO: Make sure this doesn't happen when the other thread is still active.
void wstask_deleter(SEXP wstask_xptr) {
  delete reinterpret_cast<shared_ptr<WebsocketTask>*>(R_ExternalPtrAddr(wstask_xptr));
  R_ClearExternalPtr(wstask_xptr);
}


// [[Rcpp::export]]
SEXP wsCreate(
  std::string uri,
  Rcpp::Environment robjPublic,
  Rcpp::Environment robjPrivate,
  Rcpp::CharacterVector accessLogChannels,
  Rcpp::CharacterVector errorLogChannels,
  int maxMessageSize
) {
  WebsocketTask* wstask = new WebsocketTask(uri, robjPublic, robjPrivate,
                                            accessLogChannels, errorLogChannels,
                                            maxMessageSize);


  shared_ptr<WebsocketTask> *wstask_xptr = new shared_ptr<WebsocketTask>(wstask);
  SEXP client_xptr = PROTECT(R_MakeExternalPtr(wstask_xptr, R_NilValue, R_NilValue));
  // TODO: Reinstate this.
  R_RegisterCFinalizerEx(client_xptr, wstask_deleter, TRUE);
  UNPROTECT(1);
  return client_xptr;
}

// [[Rcpp::export]]
void wsAppendHeader(SEXP client_xptr, std::string key, std::string value) {
  shared_ptr<WebsocketTask> wstask_xptr = xptrGetWsTask(client_xptr);
  wstask_xptr->client->append_header(key, value);
}

// [[Rcpp::export]]
void wsAddProtocols(SEXP client_xptr, CharacterVector protocols) {
  shared_ptr<WebsocketTask> wstask_xptr = xptrGetWsTask(client_xptr);
  for (Rcpp::CharacterVector::iterator it = protocols.begin();
       it != protocols.end();
       it++) {
    std::string protocol = Rcpp::as<std::string>(*it);
    wstask_xptr->client->add_subprotocol(protocol);
  }
}

// [[Rcpp::export]]
void wsConnect(SEXP wstask_xptr) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);
  wstask_ptr->connect();
}

// [[Rcpp::export]]
void wsRestart(SEXP wstask_xptr) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);
  wstask_ptr->client->get_io_service().restart();
}

// [[Rcpp::export]]
void wsSend(SEXP wstask_xptr, SEXP msg) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);

  if (TYPEOF(msg) == STRSXP &&
      Rf_length(msg) == 1 &&
      STRING_ELT(msg, 0) != NA_STRING)
  {
    const char* msg_ptr = CHAR(STRING_ELT(msg, 0));
    int len = R_nchar(STRING_ELT(msg, 0), Bytes, FALSE, FALSE, "wsSend");
    wstask_ptr->client->send(msg_ptr, len, ws_websocketpp::frame::opcode::text);

  } else if (TYPEOF(msg) == RAWSXP) {
    wstask_ptr->client->send(RAW(msg), Rf_length(msg), ws_websocketpp::frame::opcode::binary);
  } else {
    stop("msg must be a one-element character vector or a raw vector.");
  }
}

// [[Rcpp::export]]
void wsReset(SEXP wstask_xptr) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);
  wstask_ptr->client->reset();
}

// [[Rcpp::export]]
void wsClose(SEXP wstask_xptr, uint16_t code, std::string reason) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);
  wstask_ptr->close(code, reason);
}

// [[Rcpp::export]]
bool wsStopped(SEXP wstask_xptr) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);
  return wstask_ptr->client->stopped();
}

// [[Rcpp::export]]
std::string wsProtocol(SEXP wstask_xptr) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);
  return wstask_ptr->client->get_subprotocol();
}

// [[Rcpp::export]]
std::string wsState(SEXP wstask_xptr) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);
  switch(wstask_ptr->state) {
    case WebsocketTask::STATE::INIT: return "INIT";
    case WebsocketTask::STATE::OPEN: return "OPEN";
    case WebsocketTask::STATE::CLOSING: return "CLOSING";
    case WebsocketTask::STATE::CLOSED: return "CLOSED";
    case WebsocketTask::STATE::FAILED: return "FAILED";
  }

  // Shouldn't be possible to get here, but some compilers still complain
  // about reaching end of a non-void function.
  return "UNKNOWN";
}




// [[Rcpp::export]]
void wsUpdateLogChannels(
  SEXP wstask_xptr,
  std::string accessOrError,
  std::string setOrClear,
  Rcpp::CharacterVector logChannels
) {
  shared_ptr<WebsocketTask> wstask_ptr = xptrGetWsTask(wstask_xptr);
  wstask_ptr->client->update_log_channels(accessOrError, setOrClear, logChannels);
}
