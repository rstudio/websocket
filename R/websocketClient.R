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
    },
    connect = function() {
      private$wsObj <- wsCreate(private$url)
    },
    send = function(msg) {
      wsSend(private$wsObj, msg)
      wsPoll(private$wsObj)
    },
    close = function() {
      wsClose(private$wsObj)
    }
  ),
  private = list(
    url = NULL,
    onMessage = NULL,
    onDisconnected = NULL,
    wsObj = NULL
  )
)
