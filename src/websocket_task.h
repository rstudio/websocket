#ifndef WEBSOCKET_TASK_HPP
#define WEBSOCKET_TASK_HPP
// ============================================================================
// Websocket thread
// ============================================================================

#include <boost/noncopyable.hpp>
#include <later_api.h>
#include <Rcpp.h>
#include "websocket_defs.h"

class WebsocketTask : public later::BackgroundTask, public boost::noncopyable {
public:
  WebsocketTask(
    std::string uri,
    Rcpp::Environment robjPublic,
    Rcpp::Environment robjPrivate,
    Rcpp::CharacterVector accessLogChannels,
    Rcpp::CharacterVector errorLogChannels,
    int maxMessageSize
  );

  void rHandleMessage(message_ptr msg);
  void rHandleClose(ws_websocketpp::close::status::value code, std::string reason);
  void rHandleOpen();
  void rHandleFail();

  shared_ptr<Client> client;

  void connect();
  void close(uint16_t code, std::string reason);

  enum STATE { INIT, OPEN, CLOSING, CLOSED, FAILED };
  // TODO: Make atomic
  STATE state = INIT;

protected:
  void execute();
  void complete();

private:
  std::string uri;
  Rcpp::Environment robjPublic;
  Rcpp::Environment robjPrivate;
  Rcpp::CharacterVector accessLogChannels;
  Rcpp::CharacterVector errorLogChannels;

  // TODO: Make atomic
  bool closeOnOpen = false;

  void handleMessage(ws_websocketpp::connection_hdl, message_ptr msg);
  void handleClose(ws_websocketpp::connection_hdl);
  void handleOpen(ws_websocketpp::connection_hdl);
  void handleFail(ws_websocketpp::connection_hdl);

  void removeHandlers();

  Rcpp::Function getInvoker(std::string name);
};

#endif