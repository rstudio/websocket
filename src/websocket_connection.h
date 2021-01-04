#ifndef WEBSOCKET_CONNECTION_HPP
#define WEBSOCKET_CONNECTION_HPP

#include <boost/noncopyable.hpp>
#include <later_api.h>
#include "cpp11.hpp"
#include "websocket_defs.h"


class WebsocketConnection : public boost::noncopyable, 
                            public enable_shared_from_this<WebsocketConnection>
{
public:
  WebsocketConnection(
    std::string uri,
    int loop_id,
    cpp11::environment robjPublic,
    cpp11::environment robjPrivate,
    cpp11::strings accessLogChannels,
    cpp11::strings errorLogChannels,
    int maxMessageSize
  );

  void rHandleMessage(message_ptr msg);
  void rHandleClose(ws_websocketpp::close::status::value code, std::string reason);
  void rHandleOpen();
  void rHandleFail();

  shared_ptr<Client> client;

  void close(uint16_t code, std::string reason);

  enum STATE { INIT, OPEN, CLOSING, CLOSED, FAILED };
  // This value should be touched only from the main thread.
  STATE state = INIT;

  // ~WebsocketConnection() {
  //   std::cerr << "WebsocketConnection::~WebsocketConnection\n";
  // };

private:
  std::string uri;
  int loop_id;
  cpp11::environment robjPublic;
  cpp11::environment robjPrivate;

  // This value should be touched only from the main thread.
  bool closeOnOpen = false;

  // Callbacks for the Client object - these run on the background thread, and
  // schedule their counterparts prefixed with "r" (like rHandleMessage()) to
  // run on the main R thread.
  void handleMessage(ws_websocketpp::connection_hdl, message_ptr msg);
  void handleClose(ws_websocketpp::connection_hdl);
  void handleOpen(ws_websocketpp::connection_hdl);
  void handleFail(ws_websocketpp::connection_hdl);

  void removeHandlers();

  cpp11::function getInvoker(std::string name);
};

#endif
