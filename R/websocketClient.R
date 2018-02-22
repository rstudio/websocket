#' @useDynLib websocketClient, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @include RcppExports.R
NULL

#' @export
WebsocketClient <- R6::R6Class("WebsocketClient",
  public = list(
    initialize = function(url, onMessage = identity, onDisconnected = identity) {
      private$url <- url
      private$onMessage <- onMessage
      private$onDisconnected <- onDisconnected
      private$wsObj <- wsCreate(private$url)
      private$handleIncoming()
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
      state <- wsState(private$wsObj)
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
