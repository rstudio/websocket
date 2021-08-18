# websocket 1.4.1

1.4.0
=====

* Switched away from Rcpp and BH, to cpp11. (#73)

1.3.2
=====

* Made a tweak to autobrew in `configure` script. (@jeroen, #75)

* Added a proxy example to README.

* Made tests more reliable by eliminating need for external connections.

1.3.1
=====

* Increased OpenSSL system requirement from 1.0.1 to 1.0.2.

1.3.0
=====

* Update to WebSocket++ 0.8.2, in order to work with AsioHeaders 1.16.1-1. (#68)

* Windows and Mac: Update to OpenSSL 1.1.1g. (#69)

* Building and installing the package no longer leaves files in the system temp directory. (#69)

1.2.0
=====

* Websocket I/O now runs on a separate thread, so Websocket no longer uses polling. This should also reduce latency for handling incoming messages. (#62)

1.1.0
=====

* Added `maxMessageSize` argument to `WebSocket$new()`. (#57)

1.0.0
=====

* Initial release
