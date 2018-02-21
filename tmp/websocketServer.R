# A little example server that Winston made for testing.

library(httpuv)
cat("Starting server on port 8080...\n")
startServer("0.0.0.0", 8080,
  list(
    onWSOpen = function(ws) {
      cat("onWSOpen\n")

      ws$onMessage(function(binary, message) {
        cat("server received message:", message, "\n")
      })

      is_open <- TRUE
      count <- 0
      send_func <- function() {
        if (is_open) {
          count <<- count + 1
          ws$send(count)

          if (count == 5) {
            cat("Closing connection in 1 second\n")
            later::later(ws$close, 1)
          } else {
            later::later(send_func, 1)
          }
        }
      }
      send_func()

      ws$onClose(function() {
        is_open <<- FALSE
      })

    }
  )
)
#stopAllServers()
