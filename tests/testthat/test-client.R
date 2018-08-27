
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

  ws <- WebSocket$new(wsUrl)
  ws$onMessage(function(event) {
    expect_identical(ws, event[["target"]])
    expect_false(is.null(event[["data"]]))
    last  <<- event$data
  })
  ws$onOpen(function(event) {
    expect_identical(ws, event[["target"]])
    state <<- "open"
  })
  ws$onClose(function(event) {
    expect_identical(ws, event[["target"]])
    expect_false(is.null(event[["code"]]))
    expect_false(is.null(event[["reason"]]))
    state <<- "closed"
  })
  ws$onError(function(event) {
    expect_identical(ws, event[["target"]])
    expect_true(is.character(event[["message"]]))
    state <<- "failed"
  })

  check_later("open",
    function() !is.null(state),
    function() identical(state, "open")
  )

  # Make sure the internal state gets set, and the onOpen function gets called.
  expect_equivalent(ws$readyState(), 1L)
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
      expect_equivalent(ws$readyState(), 3L)
      expect_identical(state, "closed")
    }
  )


  expect_identical(found, 3)
}


context("Basic WebSocket")
test_that("Basic websocket communication", {
  check_ws("ws://echo.websocket.org/")
})

test_that("WebSocket object can be garbage collected", {
  collected <- FALSE
  local({
    ws <- WebSocket$new("ws://echo.websocket.org/")
    ws$onOpen(function(event) {
      ws$close()
    })
    reg.finalizer(ws, function(obj) {
      collected <<- TRUE
    })
  })
  # Pump events until there are no more; only then can we be sure that the
  # WebSocket is closed and can be garbage collected
  while (!later::loop_empty()) {
    later::run_now(1)
  }
  gc()
  expect_true(collected)
})

test_that("Open is async", {
  ws <- WebSocket$new("ws://echo.websocket.org")
  ws$onOpen(function(event) {
    ws$close()
  })
  Sys.sleep(1)
  expect_equivalent(ws$readyState(), 0L)
})

test_that("Connection errors are reported", {
  error_reported <- FALSE
  ws <- WebSocket$new("ws://example.com")
  ws$onError(function(event) {
    expect_identical(ws, event[["target"]])
    expect_true(is.character(event[["message"]]))
    expect_true(nzchar(event[["message"]]))
    error_reported <<- TRUE
  })
  while (!later::loop_empty()) {
    later::run_now(1)
  }
  expect_true(error_reported)
})

test_that("Connect can be delayed", {
  # With autoConnect = TRUE (the default), you can miss the onOpen event
  connected <- FALSE
  ws <- WebSocket$new("ws://echo.websocket.org")
  for (i in 1:20)
    later::run_now(0.1)
  ws$onOpen(function(event) {
    connected <<- TRUE
  })
  for (i in 1:20)
    later::run_now(0.1)
  expect_false(connected)
  ws$close()

  # With autoConnect = FALSE, the open event is guaranteed not to fire
  # until after connect() is called
  connected <- FALSE
  ws <- WebSocket$new("ws://echo.websocket.org", autoConnect = FALSE)
  for (i in 1:20)
    later::run_now(0.1)
  ws$connect()
  # It's OK even if onOpen is registered immediately after connect() (in the
  # same tick though), the same guarantee (that connect is asynchronous)
  # as autoConnect = TRUE applies.
  ws$onOpen(function(event) {
    connected <<- TRUE
  })
  expect_false(connected)
  while (!connected && !later::loop_empty())
    later::run_now()
  expect_true(connected)
  ws$close()
})

test_that("WebSocket can be closed before being opened or after being closed", {
  onCloseCalled <- FALSE
  ws <- WebSocket$new("ws://echo.websocket.org")
  ws$close()
  ws$onClose(function(event) {
    onCloseCalled <<- TRUE
  })
  while (!later::loop_empty())
    later::run_now()
  expect_equivalent(ws$readyState(), 3L)
  ws$close()
  expect_true(onCloseCalled)
})

test_that("WebSocket event handlers can be registered more than once", {
  a_called <- FALSE
  b_called <- FALSE
  c_called <- FALSE
  ws <- WebSocket$new("ws://echo.websocket.org")
  ws$onOpen(function(event) {
    a_called <<- TRUE
  })
  ws$onOpen(function(event) {
    b_called <<- TRUE
  })
  handle_c <- ws$onOpen(function(event) {
    c_called <<- TRUE
  })
  handle_c() # Unregister
  ws$onOpen(function(event) {
    ws$close()
  })
  while (!later::loop_empty())
    later::run_now()
  expect_true(a_called)
  expect_true(b_called)
  expect_false(c_called)
})


context("Basic SSL WebSocket")
test_that("Basic ssl websocket communication", {
  check_ws("wss://echo.websocket.org/")
})
