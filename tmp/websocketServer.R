# A little example server that Winston made for testing.

library(httpuv)
cat("Starting server on port 8080...\n")
startServer("0.0.0.0", 8080,
  list(
    onHeaders = function(req) {
      # Print connection headers
      cat(capture.output(str(as.list(req))), sep = "\n")
    },
    onWSOpen = function(ws) {
      cat("Connection opened.\n")

      ws$onMessage(function(binary, message) {
        cat("Server received message:", message, "\n")
        ws$send(message)
      })
      ws$onClose(function() {
        cat("Connection closed.\n")
      })

    }
  )
)
#stopAllServers()
