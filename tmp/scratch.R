
ws <- WebsocketClient$new("ws://localhost:8080/",
  onMessage = function(msg) cat(msg),
  onDisconnected = function() "you're disconnected"
)

ws$connect()
ws$send("whoa")
ws$close()
