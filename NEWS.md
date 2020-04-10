1.2.0
==========

* Websocket I/O now runs on a separate thread, so Websocket no longer uses polling. This should also reduce latency for handling incoming messages. ([#62](https://github.com/rstudio/websocket/pull/62))

1.1.0
=====

* Added `maxMessageSize` argument to `WebSocket$new()`. ([#57](https://github.com/rstudio/websocket/pull/57))

1.0.0
=====

* Initial release
