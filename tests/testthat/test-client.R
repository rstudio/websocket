echo_server <- function(port = httpuv::randomPort()) {
  httpuv::startServer("127.0.0.1", port,
    list(
      onWSOpen = function(ws) {
        ws$onMessage(function(binary, message) {
          ws$send(message)
        })
      }
    )
  )
}
shut_down_server <- function(s) {
  # Run the event loop a few more times to make sure httpuv handles the closed
  # websocket properly.
  for (i in 1:5) later::run_now(0.02)
  s$stop()
}
server_url <- function(server) {
  paste0("ws://", server$getHost(), ":", server$getPort(), "/")
}


test_that("Connection can't be defined with invalid maxMessageSize", {
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  expect_error(WebSocket$new(url, maxMessageSize=-1), "maxMessageSize must be a non-negative integer")
  expect_error(WebSocket$new(url, maxMessageSize=1:2), "maxMessageSize must be a non-negative integer")
})


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

test_that("small maxMessageSizes break simple connections", {
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  state <- NULL
  didFail <- FALSE

  ws <- WebSocket$new(url, maxMessageSize=2)
  ws$onMessage(function(event) {

  })
  ws$onOpen(function(event) {
    state <<- "open"
  })
  ws$onClose(function(event) {
    state <<- "closed"
  })
  ws$onError(function(event) {
    state <<- "failed"
    didFail <<- TRUE
  })

  check_later("open",
              function() !is.null(state),
              function() identical(state, "open")
  )

  # Make sure the internal state gets set, and the onOpen function gets called.
  expect_equivalent(ws$readyState(), 1L)
  expect_identical(state, "open")

  ws$send("hello")
  check_later("open",
              function() !identical(state, "open"),
              function() didFail
  )
  ws$close()
})

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
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  check_ws(url)
})

test_that("WebSocket object can be garbage collected", {
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  # Test closed WebSocket is GC'd
  collected <- FALSE
  local({
    ws <- WebSocket$new(url)
    ws$onOpen(function(event) {
      ws$close()
    })
    reg.finalizer(ws, function(obj) {
      collected <<- TRUE
    })
    # Pump events until connection is closed, or up to 10 seconds.
    end_time <- as.numeric(Sys.time()) + 10
    while (ws$readyState() != 3L && as.numeric(Sys.time()) < end_time) {
      later::run_now(0.1)
    }
  })
  gc()
  expect_true(collected)

  # Test WebSocket with failed connection is GC'd
  collected <- FALSE
  local({
    ws <- WebSocket$new("ws://example.com")
    reg.finalizer(ws, function(obj) {
      collected <<- TRUE
    })
    # Pump events until connection is closed, or up to 10 seconds.
    end_time <- as.numeric(Sys.time()) + 10
    while (ws$readyState() != 3L && as.numeric(Sys.time()) < end_time) {
      later::run_now(0.1)
    }
  })
  gc()
  expect_true(collected)
})

test_that("Open is async", {
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  onOpenCalled <- FALSE
  ws <- WebSocket$new(url)
  ws$onOpen(function(event) {
    onOpenCalled <<- TRUE
    ws$close()
  })
  Sys.sleep(1)
  # Even though the I/O happens on a separate thread, the callback which invokes
  # onOpen and sets readyState runs on the main thread using later(), so without
  # a run_now(), there would be no opportunity for these values to be changed.
  expect_equivalent(ws$readyState(), 0L)
  expect_false(onOpenCalled)

  # Run events until closed state, or timeout.
  end_time <- as.numeric(Sys.time()) + 10
  while (ws$readyState() != 3L && as.numeric(Sys.time()) < end_time) {
    later::run_now(0.1)
  }
  expect_equivalent(ws$readyState(), 3L)
  expect_true(onOpenCalled)
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

  end_time <- as.numeric(Sys.time()) + 10
  while (ws$readyState() != 3L && as.numeric(Sys.time()) < end_time) {
    later::run_now(0.1)
  }
  expect_true(error_reported)
})

test_that("Connect can be delayed", {
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  # With autoConnect = TRUE (the default), you can miss the onOpen event
  connected <- FALSE
  ws <- WebSocket$new(url)
  end_time <- as.numeric(Sys.time()) + 10
  while (ws$readyState() == 0L && as.numeric(Sys.time()) < end_time) {
    later::run_now(0.1)
  }
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
  ws <- WebSocket$new(url, autoConnect = FALSE)
  for (i in 1:10) {
    later::run_now(0.1)
  }
  ws$connect()
  # It's OK even if onOpen is registered immediately after connect() (in the
  # same tick though), the same guarantee (that connect is asynchronous) as
  # autoConnect = TRUE applies. Note the connection is made on a separate
  # thread, so the websocket could have been open before getting to the next
  # line; however, the callback for onOpen is scheduled with later() on the main
  # thread, so it can't run until a run_now().
  ws$onOpen(function(event) {
    connected <<- TRUE
  })
  expect_false(connected)
  end_time <- as.numeric(Sys.time()) + 10
  while (!connected && as.numeric(Sys.time()) < end_time) {
    later::run_now(0.1)
  }
  expect_true(connected)
  ws$close()
})

test_that("WebSocket can be closed before fully open", {
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  onCloseCalled <- FALSE
  ws <- WebSocket$new(url)
  ws$onClose(function(event) {
    onCloseCalled <<- TRUE
  })
  ws$close()
  for (i in 1:20) {
    if (onCloseCalled)
      break
    later::run_now(0.1)
  }
  expect_equivalent(ws$readyState(), 3L)
  expect_true(onCloseCalled)

  # If no connection attempt is made, then we'll stay in the pre-connectiong
  # state, and the onClose callback won't be invoked.
  onCloseCalled <- FALSE
  ws <- WebSocket$new("ws://echo.websocket.org", autoConnect = FALSE)
  ws$onClose(function(event) {
    onCloseCalled <<- TRUE
  })
  ws$close()
  expect_equivalent(ws$readyState(), -1L)
  expect_false(onCloseCalled)
})

test_that("WebSocket event handlers can be registered more than once", {
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  a_called <- FALSE
  b_called <- FALSE
  c_called <- FALSE
  ws <- WebSocket$new(url, autoConnect = FALSE)
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
  ws$connect()
  while (ws$readyState() != 3L)
    later::run_now(1)
  expect_true(a_called)
  expect_true(b_called)
  expect_false(c_called)
})

test_that("WebSocket event handlers can run in private loop", {
  s <- echo_server()
  on.exit(shut_down_server(s))
  url <- server_url(s)

  onOpenCalled <- FALSE
  loop <- later::create_loop(parent = NULL)
  ws <- WebSocket$new(url, loop = loop)
  ws$onOpen(function(event) {
    onOpenCalled <<- TRUE
  })

  # Running main loop shouldn't cause onOpen callback to run.
  for (i in 1:20) {
    later::run_now()
  }
  expect_false(onOpenCalled)

  # Runing the private loop (for the websocket) should cause the onOpen callback
  # to run. We also need to interleave running the main loop so that the httpuv
  # server can handle the connection.
  end_time <- as.numeric(Sys.time()) + 10
  while (!onOpenCalled && as.numeric(Sys.time()) < end_time) {
    later::run_now(0.1, loop = loop)
    later::run_now(0.1)
  }
  expect_true(onOpenCalled)

  ws$close()
})

test_that("WebSocket persists after reference is gone, and can be GC'd after connection is closed", {
  # Start a websocket server app where we can send commands to ws_server.
  ws_server <- NULL
  s <- httpuv::startServer("127.0.0.1", httpuv::randomPort(),
    list(
      onWSOpen = function(ws) {
        ws_server <<- ws
      }
    )
  )
  on.exit(shut_down_server(s))
  url <- server_url(s)

  finalized <- FALSE
  ws <- WebSocket$new(url)
  reg.finalizer(ws, function(e) finalized <<- TRUE)

  end_time <- as.numeric(Sys.time()) + 10
  while (ws$readyState() == 0L && as.numeric(Sys.time()) < end_time) {
    later::run_now(0.1)
  }
  rm(ws)
  gc()
  for (i in 1:5) later::run_now(0.02)

  # Connection is still open, so WebSocket shouldn't be GC'd yet.
  expect_false(finalized)

  # If we close the connection from the other side, the WebSocket should get
  # GC'd.
  ws_server$close()
  for (i in 1:5) later::run_now(0.02)
  gc()
  expect_true(finalized)
})



context("Basic SSL WebSocket")
test_that("Basic ssl websocket communication", {
  check_ws("wss://echo.websocket.org/")
})

