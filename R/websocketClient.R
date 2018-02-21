#' @useDynLib websocketClient, .registration = TRUE
#' @importFrom Rcpp sourceCpp
#' @include RcppExports.R
NULL

WebsocketClient <- R6::R6Class("WebsocketClient",
  public = list(
    initialize = function(url, onMessage = identity, onDisconnected = identity) {
      self$url <- url
    },
    connect = function() {
      private$wsObj <- wsCreate(self$url)
    },
    send = function() {},
    onMessage = function() {},
    onDisconnected = function() {},
    close = function() {}
  ),
  private = list(
    wsObj = NULL
  )
)
