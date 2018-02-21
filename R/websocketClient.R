#' @useDynLib websocketClient, .registration = TRUE
#' @importFrom Rcpp sourceCpp
NULL

WebsocketClient <- R6::R6Class("WebsocketClient",
  public = list(
    initialize = function(url, onMessage, onDisconnected) {},
    connect = function() {},
    send = function() {},
    onMessage = function() {},
    onDisconnected = function() {},
    close = function() {}
  )
)
