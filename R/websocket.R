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
#'                      accessLogChannels = "none",
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
#'   See \url{https://www.zaphoyd.com/websocketpp/manual/reference/logging} for further explanation.
#' @param errorLogChannels A character vector of error log channels that should
#'   be displayed.  The default value is \code{NULL}, which will use default websocketpp behavior.
#'   See \url{https://www.zaphoyd.com/websocketpp/manual/reference/logging} for further explanation.
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
    poll = function() {
      wsPoll(private$wsObj)
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
        self$poll()
        later::later(private$handleIncoming, 0.01)
      }
    },
    accessLogChannelValues = c("none", "connect", "disconnect", "control", "frame_header", "frame_payload", "message_header", "message_payload", "endpoint", "debug_handshake", "debug_close", "devel", "app", "http", "fail", "access_core", "all"),
    errorLogChannelValues = c("none", "devel", "library", "info", "warn", "rerror", "fatal", "all"),
    accessLogChannels = function(channels, keepIfContainsChannel) {
      channels <- match.arg(channels, private$accessLogChannelValues, several.ok = TRUE)
      if (keepIfContainsChannel %in% channels) channels <- keepIfContainsChannel
      channels
    },
    errorLogChannels = function(channels, keepIfContainsChannel) {
      channels <- match.arg(channels, private$errorLogChannelValues, several.ok = TRUE)
      if (keepIfContainsChannel %in% channels) channels <- keepIfContainsChannel
      channels
    }
  )
)
