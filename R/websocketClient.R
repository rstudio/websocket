#' @useDynLib websocketClient, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @include RcppExports.R
NULL

#' Create a websocket client
#'
#' @section Usage:
#' \preformatted{WebsocketClient$new(url, onMessage = identity, onDisconnected = identity)}
#'
#' @param url The websocket URL.
#' @param onMessage A function called for each message received from the server.
#'   Must take a single argument, the message string.
#' @param onDisconnected A function called with no arguments when either the client
#'   or the server disconnect.
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
#'   onDisconnected = function() {
#'     cat("Client disconnected\n")
#'   }
#' )
#'
#' # Send a message
#' ws$send("hello")
#'
#' # Close the websocket after we're done with it
#' ws$close()
NULL

#' @export
WebsocketClient <- R6::R6Class("WebsocketClient",
  public = list(
    initialize = function(url, onMessage = identity, onDisconnected = function() {}) {
      private$url <- url
      private$onMessage <- onMessage
      private$onDisconnected <- onDisconnected
      private$wsObj <- wsCreate(private$url)
      private$handleIncoming()
    },
    getState = function() {
      wsState(private$wsObj)
    },
    send = function(msg) {
      wsSend(private$wsObj, msg)
    },
    close = function() {
      wsClose(private$wsObj)
    }
  ),
  private = list(
    handleIncoming = function() {
      state <- self$getState()
      if (state == "CLOSED") {
        private$onDisconnected()
        return()
      } else if (state == "OPEN") {
        wsReceive(private$wsObj, private$onMessage)
      }
      # If the state is CONNECTING or CLOSING the only thing we do is recur.
      later::later(private$handleIncoming, 0.01)
    },
    url = NULL,
    onMessage = NULL,
    onDisconnected = NULL,
    wsObj = NULL
  )
)
