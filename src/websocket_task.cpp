#include "websocket_defs.h"
#include "websocket_task.h"

WebsocketTask::WebsocketTask(shared_ptr<WebsocketConnection> wsc) {
  this->wsc = wsc;
}

void WebsocketTask::execute() {
  // ASSERT_BACKGROUND_THREAD()
  wsc->client->run();
}

void WebsocketTask::complete() {
  // ASSERT_MAIN_THREAD()
}
