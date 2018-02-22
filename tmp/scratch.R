
ws <- WebsocketClient$new("ws://localhost:8080/")
ws$connect()
ws$send("whoa")
ws$close()
