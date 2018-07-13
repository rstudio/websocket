
check_later <- function(
  # debugging name
  name,
  # checks for validity
  is_valid_fn,
  # ran on valid situation
  on_success_fn,
  # ~ 30 seconds total
  counter = round(30 / sleepTime),
  # check every 0.01 seconds
  sleepTime = 0.01
) {

  found <- FALSE
  check_fn <- function() {
    counter <<- counter - 1
    Sys.sleep(sleepTime)
    if (is_valid_fn()) {
      found <<- TRUE
    }
  }

  while(counter > 0 && (!found)) {
    later::later(check_fn)
    later::run_now(5)
  }

  if (found) {
    on_success_fn()
    return(1)
  } else {
    fail(paste0("Ran out of attempts at checking: ", name))
    return(0)
  }
}


check_ws <- function(wsUrl) {
  state <- NULL
  last <- NULL
  found <- 0

  ws <- WebsocketClient$new(wsUrl,
    onMessage = function(msg) last  <<- msg,
    onOpen    = function()    state <<- "open",
    onClose   = function()    state <<- "closed",
    onFail    = function()    state <<- "failed"
  )

  # Make sure the internal state gets set, and the onOpen function gets called.
  expect_identical(ws$getState(), "OPEN")
  expect_identical(state, "open")

  last <- NULL
  ws$send("hello")
  found <- found + check_later("hello",
    function() !is.null(last),
    function() expect_identical(last, "hello")
  )


  last <- NULL
  ws$send(charToRaw("hello"))
  found <- found + check_later("hello_raw",
    function() !is.null(last),
    function() expect_identical(last, charToRaw("hello"))
  )

  state <- NULL
  ws$close()
  found <- found + check_later("closing",
    function() !is.null(state),
    function() {
      expect_identical(ws$getState(), "CLOSED")
      expect_identical(state, "closed")
    }
  )


  expect_identical(found, 3)
}


context("Basic WebSocket")
test_that("Basic websocket communication", {
  check_ws("ws://echo.websocket.org/")
})


context("Basic SSL WebSocket")
test_that("Basic ssl websocket communication", {
  check_ws("wss://echo.websocket.org/")
})
