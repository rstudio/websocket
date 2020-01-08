#ifndef WEBSOCKET_TASK_HPP
#define WEBSOCKET_TASK_HPP

#include <boost/noncopyable.hpp>
#include <later_api.h>
#include "websocket_defs.h"
#include "websocket_connection.h"

class WebsocketTask : public later::BackgroundTask,
                      public boost::noncopyable
{
public:
  WebsocketTask(shared_ptr<WebsocketConnection> wsc);

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
