#include <Rcpp.h>
#include <easywsclient.hpp>
using namespace Rcpp;
using namespace easywsclient;
using easywsclient::WebSocket;

// [[Rcpp::export]]
XPtr<WebSocket> wsCreate(std::string url) {
  WebSocket *ws = WebSocket::from_url(url);
  return XPtr<WebSocket>(ws);
}

// [[Rcpp::export]]
void wsSend(XPtr<WebSocket> ws_xptr, std::string msg) {
  WebSocket *ws = ws_xptr.get();
  ws->send(msg);
}

// [[Rcpp::export]]
void wsPoll(XPtr<WebSocket> ws_xptr) {
  WebSocket *ws = ws_xptr.get();
  ws->poll();
}

// [[Rcpp::export]]
void wsClose(XPtr<WebSocket> ws_xptr) {
  WebSocket *ws = ws_xptr.get();
  ws->close();
  // TODO Need to free XPtr and delete WebSocket
}
