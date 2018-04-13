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
#' \preformatted{WebsocketClient$new(url, onMessage,
#'                      onOpen = function() {},
#'                      onClose = function() {},
#'                      onFail = function() {})
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
#'
#'
#' @name WebsocketClient
#'
#' @examples
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
#'
NULL

#' @export
WebsocketClient <- R6::R6Class("WebsocketClient",
  public = list(
    initialize = function(url,
      onMessage = NULL,
      onOpen = function() {},
      onClose = function() {},
      onFail = function() {},
      headers = NULL
    ) {
      private$wsObj <- wsCreate(url, onMessage, onOpen, onClose, onFail)

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
    }
  ),
  private = list(
    handleIncoming = function() {
      if (self$getState() %in% c("CLOSED", "FAILED")) {
        return()
      } else {
        wsPoll(private$wsObj)
        later::later(private$handleIncoming, 0.01)
      }
    },
    wsObj = NULL
  )
)
