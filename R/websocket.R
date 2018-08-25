#' @useDynLib websocket, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @include RcppExports.R
NULL

# Used to "null out" handler functions after a websocket client is closed
null_func <- function(...) { }

#' Create a WebSocket client
#'
#' @details
#'
#' A WebSocket object has the following callback fields for you to assign your
#' own functions to. Each of these functions should take a single `event`
#' argument. The `event` argument is a named list that always contains a
#' `target` element that is the WebSocket object that originated the event, plus
#' any other relevant data as detailed below.
#'
#' \describe{
#'   \item{\code{onMessage}}{A function called for each message received from
#'     the server. The event will have a `data` element, which is the message
#'     content. If the message is text, the `data` will be a one-element
#'     character vector; if the message is binary, it will be a raw vector.}
#'   \item{\code{onOpen}}{A function called when the connection is established.}
#'   \item{\code{onClose}}{A function called when a previously-opened connection
#'     is closed. The event will have `code` (integer) and `reason` (one-element
#'     character) elements that describe the remote's reason for closing.}
#'   \item{\code{onError}}{A function called when the connection fails while the
#'     handshake is bring processed. The event will have an `message` element
#'     that is a one-element character vector describing the reason for the
#'     error.}
#' }
#'
#' It also has the following methods:
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
#' @usage
#' \preformatted{WebSocket$new(url,
#'   protocols = character(0),
#'   headers = NULL,
#'   autoConnect = TRUE,
#'   accessLogChannels = c("none"),
#'   errorLogChannels = NULL)
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
#' @param accessLogChannels A character vector of access log channels that should
#'   be enabled.  Defaults to \code{"none"}, which displays no normal, websocketpp logging activity.
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
#'   All logging levels are explained in more detail at \url{https://www.zaphoyd.com/websocketpp/manual/reference/logging}.
#' @param errorLogChannels A character vector of error log channels that should
#'   be displayed.  The default value is \code{NULL}, which will use default websocketpp behavior.
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
#'   All logging levels are explained in more detail at \url{https://www.zaphoyd.com/websocketpp/manual/reference/logging}.
#'
#'
#' @name WebSocket
#'
#' @examples
#' ## Only run this example in interactive R sessions
#' if (interactive()) {
#'
#' # Create a websocket using the websocket.org test server
#' ws <- WebSocket$new("ws://echo.websocket.org/",
#'   onMessage = function(msg) {
#'     cat("Client got msg: ", msg, "\n")
#'   },
#'   onClose = function() {
#'     cat("Client disconnected\n")
#'   }
#' )
#'
#' # Send a message
#' ws$send("hello")
#'
#' # Close the websocket after we're done with it
#' ws$close()
#' }
NULL

#' @export
WebSocket <- R6::R6Class("WebSocket",
  public = list(
    onMessage = NULL,
    onOpen = NULL,
    onClose = NULL,
    onError = NULL,
    initialize = function(url,
      protocols = character(0),
      headers = NULL,
      autoConnect = TRUE,
      accessLogChannels = c("none"),
      errorLogChannels = c("none")
    ) {
      self$onOpen <-
        self$onClose <-
        self$onError <-
        self$onMessage <-
        function(event) {}

      private$wsObj <- wsCreate(
        url, self,
        private$accessLogChannels(accessLogChannels, "none"),
        private$errorLogChannels(errorLogChannels, "none")
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
        private$scheduleIncoming()
      } else {
        warning("Ignoring extraneous connect() call (did you mean to have autoConnect=FALSE in the constructor?)")
      }
    },
    readyState = function() {
      if (private$pendingConnect) {
        return(-1)
      }

      switch(wsState(private$wsObj),
        INIT = 0L,
        OPEN = 1L,
        CLOSING = 2L,
        CLOSED = 3L,
        FAILED = 3L,
        stop("Unknown state ", wsState(private$wsObj))
      )
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
    pendingConnect = TRUE,
    scheduleIncoming = function() {
      later::later(private$handleIncoming, 0.01)
    },
    handleIncoming = function() {
      if (self$readyState() == 3L) {
        return()
      } else {
        wsPoll(private$wsObj)
        private$scheduleIncoming()
      }
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
