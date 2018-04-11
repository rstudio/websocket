#' @useDynLib websocketClient, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @include RcppExports.R
NULL

#' Create a websocket client
#'
#' @section Usage:
#' \preformatted{WebsocketClient$new(url, onMessage = identity, onClose = function() {})}
#'
#' @param url The websocket URL.
#' @param onMessage A function called for each message received from the server.
#'   Must take a single argument, the message string.
#' @param onClose A function called with no arguments when either the client or
#'   the server disconnect.
#'
#' @return a WebsocketClient instance
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
#' @export
WebsocketClient <- R6::R6Class("WebsocketClient",
  public = list(
    initialize = function(url,
      onMessage = identity,
      onOpen = function() {},
      onClose = function() {},
      onFail = function() {}
    ) {
      private$wsObj <- wsCreate(url, onMessage, onOpen, onClose, onFail)
      wsConnect(private$wsObj)
      private$handleIncoming()
    },
    getState = function() {
      wstate(private$wsObj)
    },
    send = function(msg) {
      wsend(private$wsObj, msg)
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
