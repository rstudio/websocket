#' @useDynLib websocket, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @include RcppExports.R
NULL

#' Create a websocket client
#'
#' @details
#'
#' A WebsocketClient object has the following methods:
#'
#' \describe{
#'   \item{\code{send(msg)}}{Sends a message to the server.}
#'   \item{\code{close()}}{Closes the connection.}
#'   \item{\code{getState()}}{Returns a string representing the state of the
#'     connection. One of "INIT", "OPEN", "CLOSED", "FAILED".}
#' }
#'
#' @section Usage:
#' \preformatted{WebsocketClient$new(url,
#'                      onMessage = function(msg) {},
#'                      onOpen = function() {},
#'                      onClose = function() {},
#'                      onFail = function() {},
#'                      headers = NULL,
#'                      accessLogChannels = c("none"),
#'                      errorLogChannels = NULL)
#' }
#'
#' @param url The websocket URL.
#' @param onMessage A function called for each message received from the server.
#'   Must take a single argument, the message content. If the message is text,
#'   the \code{onMessage} function will be passed a one-element character vector;
#'   if the message is binary, the \code{onMessage} function will be passed a raw
#'   vector.
#' @param onOpen A function called with no arguments when the connection is
#'   established.
#' @param onClose A function called with no arguments when either the client or
#'   the server disconnect.
#' @param onFail A function called with no arguments when the connection fails
#'   while the handshake is bring processed.
#' @param headers A named list or character vector representing keys and values
#'   of headers in the initial HTTP request.
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
#' @name WebsocketClient
#'
#' @examples
#' ## Only run this example in interactive R sessions
#' if (interactive()) {
#'
#' # Create a websocket using the websocket.org test server
#' ws <- WebsocketClient$new("ws://echo.websocket.org/",
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
WebsocketClient <- R6::R6Class("WebsocketClient",
  public = list(
    initialize = function(url,
      onMessage = function(msg) {},
      onOpen = function() {},
      onClose = function() {},
      onFail = function() {},
      headers = NULL,
      accessLogChannels = c("none"),
      errorLogChannels = NULL
    ) {
      private$wsObj <- wsCreate(
        url,
        onMessage, onOpen, onClose, onFail,
        private$accessLogChannels(accessLogChannels, "none"),
        private$errorLogChannels(errorLogChannels, "none")
      )

      mapply(names(headers), headers, FUN = function(key, value) {
        wsAppendHeader(private$wsObj, key, value)
      })

      wsConnect(private$wsObj)
      private$handleIncoming()
    },
    getState = function() {
      wsState(private$wsObj)
    },
    send = function(msg) {
      wsSend(private$wsObj, msg)
      # TODO Call wsPoll here?
    },
    close = function() {
      wsClose(private$wsObj)
      # TODO Call wsPoll here?
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
    handleIncoming = function() {
      if (self$getState() %in% c("CLOSED", "FAILED")) {
        return()
      } else {
        wsPoll(private$wsObj)
        later::later(private$handleIncoming, 0.01)
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
