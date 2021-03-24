#ifndef WEBSOCKET_TASK_HPP
#define WEBSOCKET_TASK_HPP

#include <later_api.h>
#include "websocket_defs.h"
#include "websocket_connection.h"

class WebsocketTask : public later::BackgroundTask
{
public:
  WebsocketTask(shared_ptr<WebsocketConnection> wsc);

  // Make noncopyable (without boost)
  WebsocketTask(const WebsocketTask&) = delete;
  WebsocketTask& operator=(const WebsocketTask&) = delete;

  // ~WebsocketTask() {
  //   std::cerr << "WebsocketTask::~WebsocketTask\n";
  // };

protected:
  void execute();
  void complete();

private:
  shared_ptr<WebsocketConnection> wsc;
};

#endif
