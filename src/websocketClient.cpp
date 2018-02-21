#include <Rcpp.h>
#include <easywsclient.hpp>
using namespace Rcpp;
using easywsclient::WebSocket;

// [[Rcpp::export]]
NumericVector timesTwo(NumericVector x) {
  return x * 2;
}

// [[Rcpp::export]]
NumericVector wsCreate(std::string url) {
  WebSocket *ws = WebSocket::from_url(url);
  ws->send("goodbye");
  ws->send("hello");
  // Poll flushes the tx queue in addition to handling received messages.
  ws->poll();
  delete ws;
  return NumericVector::create(1,2,3);
}

