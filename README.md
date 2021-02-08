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

Note that if you want to `send()` a message as soon as it connects, without having to wait at the console, you can put the `ws$send()` inside of the `ws$onOpen` callback, as in:

```R
ws <- WebSocket$new("wss://echo.websocket.org/")
ws$onMessage(function(event) {
  cat("Client got msg: ", event$data, "\n")
})
ws$onOpen(function(event) {
  ws$send("hello")
})
```

## Examples


### WebSocket proxy

The websocket (client) package can be used with a WebSocket server, implemented as a httpuv web application, to act as a WebSocket proxy. This proxy can be modified do things like log the traffic in each direction, or
modify messages sent in either direction.

The proxy will listen to port 5002 on the local machine, and connect to the remote machine at wss://echo.websocket.org:80. In this example, after starting the proxy, we'll connect to it with a WebSocket client in a separate R process, then send a message from that process. Here's what will happen in the proxy:

* The client will send the message `"hello"`.
* The proxy will receive `"hello"` from the client, convert it to `"HELLO"`, then send it to the remote echo server.
* The echo server will receive `"HELLO"` and send it back.
* The proxy will receive `"HELLO"` from the server, convert it to `"HELLO, world"`, then send it to the client.
* The client will receive `"HELLO, world"`.

To run this proxy, run the code below in an R session:

```R
library(curl)
library(httpuv)
library(websocket)

# URL of the remote websocket server
target_host <- "echo.websocket.org:80"
# Should be "ws" or "wss"
target_protocol <- "ws"

# Port this computer will listen on
listen_port <- 5002

# ==============================================================================
# Functions for translating header strings between HTTP request and Rook formats
# ==============================================================================
req_rook_to_curl <- function(req, host) {
  # Rename headers. Example: HTTP_CACHE_CONTROL => Cache-Control
  r <- as.list(req)

  # Uncomment to print out request headers
  # cat("== Original ==\n")
  # cat(capture.output(print(str(r))), sep = "\n")

  r <- r[grepl("^HTTP_", names(r))]
  nms <- names(r)
  nms <- sub("^HTTP_", "", nms)
  nms <- tolower(nms)
  nms <- gsub("_", "-", nms, fixed = TRUE)
  nms <- gsub("\\b([a-z])", "\\U\\1", nms, perl = TRUE)
  names(r) <- nms
  # Overwrite host field
  r$Host <- host

  # Uncomment to print out modified request headers
  # cat("== Modified ==\n")
  # cat(capture.output(print(str(r))), sep = "\n")
  r
}

resp_httr_to_rook <- function(resp) {
  status <- as.integer(sub("^HTTP\\S+ (\\d+).*", "\\1", curl::parse_headers(resp$headers)[1]))
  list(
    status = status,
    headers = parse_headers_list(resp$headers),
    body = resp$content
  )
}


# ==============================================================================
# Websocket proxy app
# ==============================================================================

# These functions are called from the server app; defined here so that they
# can be modified while the application is running.
onHeaders <- function(req) {
  # Print out the headers received from server
  # str(as.list(req$HEADERS))
  NULL
}

call <- function(req) {
  req_curl <- req_rook_to_curl(req, target_host)
  h <- new_handle()
  do.call(handle_setheaders, c(h, req_curl))
  resp_curl <- curl_fetch_memory(paste0("http://", target_host, req$PATH_INFO), handle = h)
  resp_httr_to_rook(resp_curl)
}

onWSOpen <- function(clientWS) {
  # The httpuv package contains a WebSocket server class and the websocket
  # package contains a WebSocket client class. It may be a bit confusing, but
  # both of these classes are named "WebSocket", and they have slightly
  # different interfaces.
  serverWS <- websocket::WebSocket$new(paste0(target_protocol, "://", target_host))

  msg_from_client_buffer <- list()
  # Flush the queued messages from the client
  flush_msg_from_client_buffer <- function() {
    for (msg in msg_from_client_buffer) {
      serverWS$send(msg)
    }
    msg_from_client_buffer <<- list()
  }
  clientWS$onMessage(function (isBinary, msgFromClient) {
    cat("Got message from client: ", msgFromClient, "\n")

    # NOTE: This is where we modify the messages going from the client to the
    # server. This simply converts to upper case. You can modify to suit your
    # needs.
    msgFromClient <- toupper(msgFromClient)
    cat("Converting toupper() and then sending to server: ", msgFromClient, "\n")

    if (serverWS$readyState() == 0) {
      msg_from_client_buffer[length(msg_from_client_buffer) + 1] <<- msgFromClient
    } else {
      serverWS$send(msgFromClient)
    }
  })
  serverWS$onOpen(function(event) {
    serverWS$onMessage(function(msgFromServer) {
      cat("Got message from server: ", msgFromServer$data, "\n")

      # NOTE: This is where we could modify the messages going from the server
      # to the client. You can modify to suit your needs.
      msg <- paste0(msgFromServer$data, ", world")
      cat('Appending ", world" and then sending to client: ', msg, "\n")

      clientWS$send(msg)
    })
    flush_msg_from_client_buffer()
  })
}

# Start the websocket proxy app
s <- startServer("0.0.0.0", listen_port,
  list(
    onHeaders = function(req) {
      onHeaders(req)
    },
    call = function(req) {
      call(req)
    },
    onWSOpen = function(clientWS) {
      onWSOpen(clientWS)
    }
  )
)

# Run this to stop the server:
# s$stop()

# If you want to run this code with `Rscript -e "source('server.R')"`, also
# uncomment the following so that it doesn't immediately exit.
# httpuv::service(Inf)
```

In a separate R session, you can connect to this proxy and send a message.

```R
# Connect to proxy
ws <- websocket::WebSocket$new("ws://localhost:5002")
ws$onMessage(function(event) {
  cat(paste0('Received message from proxy: "', event$data, '"\n'))
})
# Send message to proxy
ws$send("hello")
#> Received message from proxy: "HELLO, world"
```


In the R process running the proxy, you will see:

```
Got message from client:  hello
Converting toupper() and then sending to server:  HELLO
Got message from server:  HELLO
Appending ", world" and then sending to client:  HELLO, world
```



### Testing the websocket client with a WebSocket echo server

This is a simple WebSocket echo server implemented with httpuv. You can run it and interact with using the WebSocket client code below.

```R
library(httpuv)
cat("Starting server on port 8080...\n")
s <- startServer("0.0.0.0", 8080,
  list(
    onHeaders = function(req) {
      # Print connection headers
      cat(capture.output(str(as.list(req))), sep = "\n")
    },
    onWSOpen = function(ws) {
      cat("Connection opened.\n")

      ws$onMessage(function(binary, message) {
        cat("Server received message:", message, "\n")
        ws$send(message)
      })
      ws$onClose(function() {
        cat("Connection closed.\n")
      })
    }
  )
)
```


This code will connect to the echo server and send a message:

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


If you want it to send the message as soon as it connects (without having to wait for a moment at the console), you can tell it to do that in the `onOpen` callback:

```R
ws$onOpen(function(event) {
  cat("Connection opened\n")
  ws$send("hello")
})
```
