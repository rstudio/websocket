1.2.0.9000
=====

* Update to WebSocket++ 0.8.2, in order to work with AsioHeaders 1.16.1-1. (#68)

1.2.0
=====

* Websocket I/O now runs on a separate thread, so Websocket no longer uses polling. This should also reduce latency for handling incoming messages. (#62)

1.1.0
=====

* Added `maxMessageSize` argument to `WebSocket$new()`. (#57)

1.0.0
=====

* Initial release
