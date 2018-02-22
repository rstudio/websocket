ws <- WebsocketClient$new("ws://localhost:8080/",
  onMessage = function(msg) {
    cat("Client got msg: ", msg, "\n")
  },
  onDisconnected = function() {
    cat("Client disconnected\n")
  }
)

ws$send("whoa")
ws$close()
ws$connect()
