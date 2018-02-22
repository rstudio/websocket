#include <Rcpp.h>
#include <easywsclient.hpp>
using namespace Rcpp;
//using namespace easywsclient;
using easywsclient::WebSocket;

// [[Rcpp::export]]
SEXP wsCreate(std::string url) {
  WebSocket *ws = WebSocket::from_url(url);
  SEXP ws_xptr = PROTECT(R_MakeExternalPtr(ws, R_NilValue, R_NilValue));
  UNPROTECT(1);
  return ws_xptr;
}

WebSocket* xptrGetWs(SEXP ws_xptr) {
  if (TYPEOF(ws_xptr) != EXTPTRSXP) {
    throw Rcpp::exception("Expected external pointer.");
  }
  return reinterpret_cast<WebSocket*>(R_ExternalPtrAddr(ws_xptr));
}

// [[Rcpp::export]]
void wsSend(SEXP ws_xptr, std::string msg) {
  xptrGetWs(ws_xptr)->send(msg);
}

// [[Rcpp::export]]
void wsPoll(SEXP ws_xptr) {
  xptrGetWs(ws_xptr)->poll();
}

// [[Rcpp::export]]
void wsClose(SEXP ws_xptr) {
  xptrGetWs(ws_xptr)->close();
  // TODO Need to free XPtr and delete WebSocket
}
