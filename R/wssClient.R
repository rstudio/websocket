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

# ws <- websocketClient::WSSClient$new("wss://echo.websocket.org", onMessage = function(m) cat("received", m, "\n"))
#' @export
WSSClient <- R6::R6Class("WSSClient",
  public = list(
    initialize = function(url, onMessage = identity, onOpen = function() {}, onClose = function() {}, onFail = function() {}) {
      private$wssObj <- wssCreate(url, onMessage, onOpen, onClose, onFail)
      wssConnect(private$wssObj)
      private$handleIncoming()
    },
    getState = function() {
      wssState(private$wssObj)
    },
    send = function(msg) {
      wssSend(private$wssObj, msg)
      # TODO Call wssPoll here?
    },
    close = function() {
      wssClose(private$wssObj)
      # TODO Call wssPoll here?
    }
  ),
  private = list(
    handleIncoming = function() {
      if (self$getState() %in% c("CLOSED", "FAILED")) {
        return()
      } else {
        wssPoll(private$wssObj)
        later::later(private$handleIncoming, 0.01)
      }
    },
    wssObj = NULL
  )
)
