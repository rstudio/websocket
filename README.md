# websocket <img src="man/figures/websocket_logo.svg" align="right" height="250px" />

<!-- badges: start -->
[![CRAN status](https://www.r-pkg.org/badges/version/websocket)](https://cran.r-project.org/package=websocket)
[![R build status](https://github.com/rstudio/websocket/workflows/R-CMD-check/badge.svg)](https://github.com/rstudio/websocket/actions)
<!-- badges: end -->

This is an R WebSocket client library backed by the [websocketpp](https://github.com/zaphoyd/websocketpp) C++ library. WebSocket I/O is handled on a separate thread from R.

## Usage examples

```R
library(websocket)

ws <- WebSocket$new("ws://echo.websocket.org/", autoConnect = FALSE)
ws$onOpen(function(event) {
  cat("Connection opened\n")
})
ws$onMessage(function(event) {
  cat("Client got msg: ", event$data, "\n")
})
ws$onClose(function(event) {
  cat("Client disconnected with code ", event$code,
    " and reason ", event$reason, "\n", sep = "")
})
ws$onError(function(event) {
  cat("Client failed to connect: ", event$message, "\n")
})
ws$connect()
```

(If you're not writing these commands at a console—for instance, if you're using a WebSocket as part of a Shiny app—you can leave off `autoConnect=FALSE` and `ws$connect()`. These are only necessary when the creation of the WebSocket object and the registering of the `onOpen`/`onMessage`/`onClose` handlers don't happen within the same function call, because in those cases the connection may open and/or messages received before the handlers are registered.)

Once connected, you can send commands as follows:

```R
# Send text messages
ws$send("hello")

# Send binary messages
ws$send(charToRaw("hello"))

# Finish
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

Then, download the [Websocket Server script](https://github.com/rstudio/websocket/blob/master/tmp/websocketServer.R) and run this in a terminal:

```
R -e 'source("websocketServer.R"); httpuv::service(Inf)'
```

Finally, to test the WebSocket client, run this in R.

```R
library(websocket)

ws <- WebSocket$new("ws://127.0.0.1:8080/",
  headers = list(Cookie = "Xyz"),
  accessLogChannels = "all" # enable all websocketpp logging
)

ws$onOpen(function(event) {
  cat("Connection opened\n")
})
ws$onMessage(function(event) {
  cat("Client got msg: ", event$data, "\n")
})
ws$onClose(function(event) {
  cat("Client disconnected with code ", event$code,
    " and reason ", event$reason, "\n", sep = "")
})
ws$onError(function(event) {
  cat("Client failed to connect: ", event$message, "\n")
})

ws$send("hello")

ws$send(charToRaw("hello"))

ws$close()
```
