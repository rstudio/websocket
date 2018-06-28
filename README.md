websocket
=========

This is an R WebSocket client library backed by the [websocketpp](https://github.com/zaphoyd/websocketpp) C++ library.

## Usage examples

You may need to step through the `$send()` commands because there will be a small amount of time before the response is received. The `onMessage()` callback is invoked asynchronously (using the [later](https://github.com/r-lib/later) package) when a response arrives.

```R
library(websocket)

ws <- WebsocketClient$new("ws://echo.websocket.org/",
  onOpen = function() {
    cat("Connection opened\n")
  },
  onMessage = function(msg) {
    cat("Client got msg: ", msg, "\n")
  },
  onClose = function() {
    cat("Client disconnected\n")
  }
)

ws$send("hello")

ws$send( charToRaw("hello") )

ws$close()
```

websocket supports ws:// and wss:// URLs.

## Development setup

Currently we do local development by running a simple httpuv-backed WebSocket server written in R that lives at `tmp/websocketServer.R`.

To run it, you need to install the Github version of `httpuv`.

### Running testing WebSocket server on macOS

> Note: If you want httpuv to build faster, you can create a file at ~/.Renviron with the following content: `MAKEFLAGS=-j4`

To install the development version of httpuv, run:

```R
remotes::install_github("rstudio/httpuv")`
```

Then to run the WebSocket server, run this in a terminal:

```
R -e 'source("tmp/websocketServer.R"); httpuv::service(Inf)'
```

Finally, to test the WebSocket client, run this in R.

```R
library(websocket)

ws <- WebsocketClient$new("ws://127.0.0.1:8080/",
  headers = list(Cookie = "Xyz"),
  onOpen = function() {
    cat("Connection opened.\n")
  },
  onMessage = function(msg) {
    cat("Client received message: ", msg, "\n")
  },
  onClose = function() {
    cat("Connection closed.\n")
  },
  accessLogChannels = "all" # enable all websocketpp logging
)

ws$send("hello")

ws$send( charToRaw("hello") )

ws$close()
```
