#' @useDynLib websocket, .registration = TRUE
#' @import later
#' @importFrom Rcpp sourceCpp
#' @importFrom R6 R6Class
#' @include RcppExports.R
NULL

# Used to "null out" handler functions after a websocket client is closed
null_func <- function(...) { }

#' Create a WebSocket client
#'
#' \preformatted{
#' WebSocket$new(url,
#'   protocols = character(0),
#'   headers = NULL,
#'   autoConnect = TRUE,
#'   accessLogChannels = c("none"),
#'   errorLogChannels = NULL,
#'   maxMessageSize = 32 * 1024 * 1024)
#' }
#'
#' @details
#'
#' A WebSocket object has four events you can listen for, by calling the
#' corresponding `onXXX` method and passing it a callback function. All callback
#' functions must take a single `event` argument. The `event` argument is a
#' named list that always contains a `target` element that is the WebSocket
#' object that originated the event, plus any other relevant data as detailed
#' below.
#'
#' \describe{
#'   \item{\code{onMessage}}{Called each time a message is received from the
#'     server. The event will have a `data` element, which is the message
#'     content. If the message is text, the `data` will be a one-element
#'     character vector; if the message is binary, it will be a raw vector.}
#'   \item{\code{onOpen}}{Called when the connection is established.}
#'   \item{\code{onClose}}{Called when a previously-opened connection is closed.
#'     The event will have `code` (integer) and `reason` (one-element character)
#'     elements that describe the remote's reason for closing.}
#'   \item{\code{onError}}{Called when the connection fails to be established.
#'     The event will have an `message` element, a character vector of length 1
#'     describing the reason for the error.}
#' }
#'
#' Each `onXXX` method can be called multiple times to register multiple
#' callbacks. Each time an `onXXX` is called, its (invisible) return value is a
#' function that can be invoked to cancel that particular registration.
#'
#' A WebSocket object also has the following methods:
#'
#' \describe{
#'   \item{\code{connect()}}{Initiates the connection to the server. (This does
#'     not need to be called unless you have passed `autoConnect=FALSE` to the
#'     constructor.)}
#'   \item{\code{send(msg)}}{Sends a message to the server.}
#'   \item{\code{close()}}{Closes the connection.}
#'   \item{\code{readyState()}}{Returns an integer representing the state of the
#'     connection.
#'     \describe{
#'       \item{\code{0L}: Connecting}{The WebSocket has not yet established a
#'       connection with the server.}
#'       \item{\code{1L}: Open}{The WebSocket has connected and can send and
#'       receive messages.}
#'       \item{\code{2L}: Closing}{The WebSocket is in the process of closing.}
#'       \item{\code{3L}: Closed}{The WebSocket has closed, or failed to open.}
#'     }}
#'   \item{setAccessLogChannels(channels)}{Enable the websocket Access channels after the
#'     websocket's creation.  A value of \code{NULL} will not enable any new Access channels.}
#'   \item{setErrorLogChannels(channels)}{Enable the websocket Error channels after the
#'     websocket's creation.  A value of \code{NULL} will not enable any new Error channels.}
#'   \item{clearAccessLogChannels(channels)}{Disable the websocket Access channels after the
#'     websocket's creation.  A value of \code{NULL} will not clear any existing Access channels.}
#'   \item{clearErrorLogChannels(channels)}{Disable the websocket Error channels after the
#'     websocket's creation.  A value of \code{NULL} will not clear any existing Error channels.}
#' }
#'
#' @param url The WebSocket URL. Should begin with \code{ws://} or \code{wss://}.
#' @param protocols Zero or more WebSocket sub-protocol names to offer to the server
#'   during the opening handshake.
#' @param headers A named list or character vector representing keys and values
#'   of headers in the initial HTTP request.
#' @param autoConnect If set to `FALSE`, then constructing the WebSocket object
#'   will not automatically cause the connection to be established. This can be
#'   used if control will return to R before event handlers can be set on the
#'   WebSocket object (i.e. you are constructing a WebSocket object manually at
#'   an interactive R console); after you are done attaching event handlers, you
#'   must call `ws$connect()` to establish the WebSocket connection.
#' @param accessLogChannels A character vector of access log channels that are
#'   enabled.  Defaults to \code{"none"}, which displays no normal, websocketpp logging activity.
#'   Setting \code{accessLogChannels = NULL} will use default websocketpp behavior.
#'   Multiple access logging levels may be passed in for them to be enabled.
#'
#'   A few commonly used access logging values are:
#'   \describe{
#'     \item{\code{"all"}}{Special aggregate value representing "all levels"}
#'     \item{\code{"none"}}{Special aggregate value representing "no levels"}
#'     \item{\code{"rerror"}}{Recoverable error. Recovery may mean cleanly closing the connection
#'           with an appropriate error code to the remote endpoint.}
#'     \item{\code{"fatal"}}{Unrecoverable error. This error will trigger immediate unclean
#'           termination of the connection or endpoint.}
#'   }
#'
#'   All logging levels are explained in more detail at \url{https://docs.websocketpp.org/reference_8logging.html}.
#' @param errorLogChannels A character vector of error log channels that are
#'   displayed.  The default value is \code{NULL}, which will use default websocketpp behavior.
#'   Multiple error logging levels may be passed in for them to be enabled.
#'
#'   A few commonly used error logging values are:
#'   \describe{
#'     \item{\code{"all"}}{Special aggregate value representing "all levels"}
#'     \item{\code{"none"}}{Special aggregate value representing "no levels"}
#'     \item{\code{"connect"}}{One line for each new connection that is opened}
#'     \item{\code{"disconnect"}}{One line for each new connection that is closed}
#'   }
#'
#'   All logging levels are explained in more detail at \url{https://docs.websocketpp.org/reference_8logging.html}.
#' @param maxMessageSize The maximum size of a message in bytes. If a message
#'   larger than this is sent, the connection will fail with the \code{message_too_big}
#'   protocol error.
#'
#'
#' @name WebSocket
#'
#' @examples
#' ## Only run this example in interactive R sessions
#' if (interactive()) {
#'
#' # Create a websocket using the websocket.org test server
#' ws <- WebSocket$new("ws://echo.websocket.org/")
#' ws$onMessage(function(event) {
#'   cat("Client got msg:", event$data, "\n")
#' })
#' ws$onClose(function(event) {
#'   cat("Client disconnected\n")
#' })
#' ws$onOpen(function(event) {
#'   cat("Client connected\n")
#' })
#'
#' # Try sending a message with ws$send("hello").
#' # Close the websocket with ws$close() after you're done with it.
#' }
NULL

#' @export
WebSocket <- R6Class("WebSocket",
  public = list(
    initialize = function(url,
      protocols = character(0),
      headers = NULL,
      autoConnect = TRUE,
      accessLogChannels = c("none"),
      errorLogChannels = NULL,
      maxMessageSize = 32 * 1024 * 1024,
      loop = later::current_loop()
    ) {
      private$callbacks <- new.env(parent = emptyenv())
      private$callbacks$open <- Callbacks$new()
      private$callbacks$close <- Callbacks$new()
      private$callbacks$error <- Callbacks$new()
      private$callbacks$message <- Callbacks$new()

      if (length(maxMessageSize) != 1 || !is.numeric(maxMessageSize) || maxMessageSize < 0){
        stop("maxMessageSize must be a non-negative integer")
      }

      private$wsObj <- wsCreate(
        url, loop$id, self, private,
        private$accessLogChannels(accessLogChannels, "none"),
        private$errorLogChannels(errorLogChannels, "none"),
        maxMessageSize
      )

      mapply(names(headers), headers, FUN = function(key, value) {
        wsAppendHeader(private$wsObj, key, value)
      })
      wsAddProtocols(private$wsObj, protocols)

      private$pendingConnect <- TRUE
      if (autoConnect) {
        self$connect()
      }
    },
    connect = function() {
      if (private$pendingConnect) {
        private$pendingConnect <- FALSE
        wsConnect(private$wsObj)
      } else {
        warning("Ignoring extraneous connect() call (did you mean to have autoConnect=FALSE in the constructor?)")
      }
    },
    readyState = function() {
      code <- function(value, desc) {
        structure(value, description = desc)
      }

      if (private$pendingConnect) {
        return(code(-1L, "Pre-connecting"))
      }

      switch(wsState(private$wsObj),
        INIT = code(0L, "Connecting"),
        OPEN = code(1L, "Open"),
        CLOSING = code(2L, "Closing"),
        CLOSED = code(3L, "Closed"),
        FAILED = code(3L, "Closed"),
        stop("Unknown state ", wsState(private$wsObj))
      )
    },
    onOpen = function(callback) {
      invisible(private$callbacks[["open"]]$register(callback))
    },
    onClose = function(callback) {
      invisible(private$callbacks[["close"]]$register(callback))
    },
    onError = function(callback) {
      invisible(private$callbacks[["error"]]$register(callback))
    },
    onMessage = function(callback) {
      invisible(private$callbacks[["message"]]$register(callback))
    },
    protocol = function() {
      wsProtocol(private$wsObj)
    },
    send = function(msg) {
      wsSend(private$wsObj, msg)
    },
    close = function(code = 1000L, reason = "") {
      wsClose(private$wsObj, code, reason)
    },
    setAccessLogChannels = function(channels = c("all")) {
      wsUpdateLogChannels(private$wsObj, "access", "set", private$accessLogChannels(channels, "none"))
    },
    setErrorLogChannels = function(channels = c("all")) {
      wsUpdateLogChannels(private$wsObj, "error", "set", private$errorLogChannels(channels, "none"))
    },
    clearAccessLogChannels = function(channels = c("all")) {
      wsUpdateLogChannels(private$wsObj, "access", "clear", private$accessLogChannels(channels, "all"))
    },
    clearErrorLogChannels = function(channels = c("all")) {
      wsUpdateLogChannels(private$wsObj, "error", "clear", private$errorLogChannels(channels, "all"))
    }
  ),
  private = list(
    wsObj = NULL,
    callbacks = NULL,
    pendingConnect = TRUE,
    getInvoker = function(eventName) {
      callbacks <- private$callbacks[[eventName]]
      stopifnot(!is.null(callbacks))
      callbacks$invoke
    },
    accessLogChannelValues = c(
      "none", "connect", "disconnect", "control", "frame_header", "frame_payload",
      "message_header", "message_payload", "endpoint", "debug_handshake", "debug_close", "devel",
      "app", "http", "fail", "access_core", "all"
    ),
    errorLogChannelValues = c("none", "devel", "library", "info", "warn", "rerror", "fatal", "all"),
    accessLogChannels = function(channels, stompValue) {
      if (is.null(channels)) return(character(0))
      channels <- match.arg(channels, private$accessLogChannelValues, several.ok = TRUE)
      if (stompValue %in% channels) channels <- stompValue
      channels
    },
    errorLogChannels = function(channels, stompValue) {
      if (is.null(channels)) return(character(0))
      channels <- match.arg(channels, private$errorLogChannelValues, several.ok = TRUE)
      if (stompValue %in% channels) channels <- stompValue
      channels
    }
  )
)

Callbacks <- R6Class(
  'Callbacks',
  private = list(
    .nextId = integer(0),
    .callbacks = 'environment'
  ),
  public = list(
    initialize = function() {
      # NOTE: we avoid using '.Machine$integer.max' directly
      # as R 3.3.0's 'radixsort' could segfault when sorting
      # an integer vector containing this value
      private$.nextId <- as.integer(.Machine$integer.max - 1L)
      private$.callbacks <- new.env(parent = emptyenv())
    },
    register = function(callback) {
      if (!is.function(callback)) {
        stop("callback must be a function")
      }
      id <- as.character(private$.nextId)
      private$.nextId <- private$.nextId - 1L
      private$.callbacks[[id]] <- callback
      return(function() {
        rm(list = id, pos = private$.callbacks)
      })
    },
    invoke = function(...) {
      # Ensure that calls are invoked in the order that they were registered
      keys <- as.character(sort(as.integer(ls(private$.callbacks)), decreasing = TRUE))
      callbacks <- mget(keys, private$.callbacks)

      for (callback in callbacks) {
        tryCatch(
          callback(...),
          error = function(e) {
            message("Error in websocket callback: ", e$message)
          },
          interrupt = function(e) {
            message("Interrupt received while executing websocket callback.")
          }
        )

      }
    },
    count = function() {
      length(ls(private$.callbacks))
    }
  )
)
