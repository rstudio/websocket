# This contains an WebSocket proxy, implemented as a httpuv web application,
# with websocket. This proxy can be modified to log the traffic in each
# direction, and it messages sent through this proxy can be modified in either
# direction.
#
# This proxy will listen to port 5002 on the local machine, and connect to the
# remote machine at ws://echo.websocket.org:80. In this example, the outgoing
# messages will be converted to upper case, then sent to the WebSocket echo
# server. Messages from the echo server will be sent to the client unchanged.
#
# To run this proxy, run the code from this file in one R session. Then, in
# another R session, run the following to test the proxy:
#
#   # Connect to proxy
#   ws <- websocket::WebSocket$new("ws://localhost:5002")
#   ws$onMessage(function(event) {
#     cat(paste0('Received message from proxy: "', event$data, '"\n'))
#   })
#   # Send message to proxy
#   ws$send("hello")
#
#
# In this R session, you will see:
#
#   Got message from client:  hello
#   Converting toupper() and then sending to server:  HELLO
#   Got message from server:  HELLO
#   Appending ", world" and then sending to client:  HELLO, world


library(curl)
library(httpuv)
library(websocket)

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

# URL of the remote websocket server
target_host <- "echo.websocket.org:80"
# Port this computer will listen on
listen_port <- 5002

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
  serverWS <- websocket::WebSocket$new(paste0("ws://", target_host))

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


# Stop the server
#stopServer(s)
