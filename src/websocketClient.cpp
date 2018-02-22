#include <Rcpp.h>
#include <easywsclient.hpp>
using namespace Rcpp;
using easywsclient::WebSocket;

WebSocket* xptrGetWs(SEXP ws_xptr) {
  if (TYPEOF(ws_xptr) != EXTPTRSXP) {
    throw Rcpp::exception("Expected external pointer.");
  }
  return reinterpret_cast<WebSocket*>(R_ExternalPtrAddr(ws_xptr));
}

void websocket_deleter(SEXP ws_xptr) {
  delete xptrGetWs(ws_xptr);
  R_ClearExternalPtr(ws_xptr);
  Rprintf("Deleted the websocket\n");
}

// [[Rcpp::export]]
SEXP wsCreate(std::string url) {
  WebSocket *ws = WebSocket::from_url(url);
  SEXP ws_xptr = PROTECT(R_MakeExternalPtr(ws, R_NilValue, R_NilValue));
  R_RegisterCFinalizerEx(ws_xptr, websocket_deleter, TRUE);
  UNPROTECT(1);
  return ws_xptr;
}

// [[Rcpp::export]]
void wsSend(SEXP ws_xptr, std::string msg) {
  WebSocket *ws = xptrGetWs(ws_xptr);
  ws->send(msg);
  ws->poll();
}

// [[Rcpp::export]]
void wsReceive(SEXP ws_xptr, Rcpp::Function onMessage) {
  WebSocket *ws = xptrGetWs(ws_xptr);
  ws->poll();
  ws->dispatch(onMessage);
}

// [[Rcpp::export]]
void wsClose(SEXP ws_xptr) {
  WebSocket *ws = xptrGetWs(ws_xptr);
  ws->close();
}

// [[Rcpp::export]]
std::string wsState(SEXP ws_xptr) {
  WebSocket *ws = xptrGetWs(ws_xptr);
  ws->poll();
  WebSocket::readyStateValues state = ws->getReadyState();
  switch(state) {
    case WebSocket::readyStateValues::CLOSED: return "CLOSED";
    case WebSocket::readyStateValues::CLOSING: return "CLOSING";
    case WebSocket::readyStateValues::CONNECTING: return "CONNECTING";
    case WebSocket::readyStateValues::OPEN: return "OPEN";
  }
}
