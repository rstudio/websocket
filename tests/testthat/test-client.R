context("client")

test_that("Basic websocket communication", {
  state <- NULL
  last <- NULL
  found <- 0

  ws <- WebsocketClient$new("ws://echo.websocket.org/",
    onMessage = function(msg) last  <<- msg,
    onOpen    = function()    state <<- "open",
    onClose   = function()    state <<- "closed",
    onFail    = function()    state <<- "failed"
  )

  # Make sure the internal state gets set, and the onOpen function gets called.
  expect_identical(ws$getState(), "OPEN")
  expect_identical(state, "open")

  hello_fn <- function() {
    Sys.sleep(0.1)
    if (!is.null(last)) {
      counter <<- 0
      expect_identical(last, "hello")
      found <<- found + 1
    } else {
      counter <<- counter - 1
    }
  }
  counter <- 20
  last <- NULL
  ws$send("hello")
  while(counter > 0) {
    later::later(hello_fn)
    later::run_now(2)
  }


  hello_raw_fn <- function() {
    Sys.sleep(0.1)
    if (!is.null(last)) {
      counter <<- 0
      expect_identical(last, charToRaw("hello"))
      found <<- found + 1
    } else {
      counter <<- counter - 1
    }
  }
  counter <- 20
  last <- NULL
  ws$send(charToRaw("hello"))
  while(counter > 0) {
    later::later(hello_raw_fn)
    later::run_now(2)
  }


  close_fn <- function() {
    Sys.sleep(0.1)
    if (!is.null(state)) {
      counter <<- 0
      expect_identical(ws$getState(), "CLOSED")
      expect_identical(state, "closed")
      found <<- found + 2
    } else {
      counter <<- counter - 1
    }
  }
  counter <- 20
  state <- NULL
  ws$close()
  while(counter > 0) {
    later::later(close_fn)
    later::run_now(2)
  }

  expect_identical(found, 4)

})


test_that("Basic ssl websocket communication", {
  state <- NULL
  last <- NULL
  found <- 0

  ws <- WebsocketClient$new("wss://echo.websocket.org/",
    onMessage = function(msg) last  <<- msg,
    onOpen    = function()    state <<- "open",
    onClose   = function()    state <<- "closed",
    onFail    = function()    state <<- "failed"
  )

  # Make sure the internal state gets set, and the onOpen function gets called.
  expect_identical(ws$getState(), "OPEN")
  expect_identical(state, "open")

  hello_fn <- function() {
    Sys.sleep(0.1)
    if (!is.null(last)) {
      counter <<- 0
      expect_identical(last, "hello")
      found <<- found + 1
    } else {
      counter <<- counter - 1
    }
  }
  counter <- 20
  last <- NULL
  ws$send("hello")
  while(counter > 0) {
    later::later(hello_fn)
    later::run_now(2)
  }


  hello_raw_fn <- function() {
    Sys.sleep(0.1)
    if (!is.null(last)) {
      counter <<- 0
      expect_identical(last, charToRaw("hello"))
      found <<- found + 1
    } else {
      counter <<- counter - 1
    }
  }
  counter <- 20
  last <- NULL
  ws$send(charToRaw("hello"))
  while(counter > 0) {
    later::later(hello_raw_fn)
    later::run_now(2)
  }


  close_fn <- function() {
    Sys.sleep(0.1)
    if (!is.null(state)) {
      counter <<- 0
      expect_identical(ws$getState(), "CLOSED")
      expect_identical(state, "closed")
      found <<- found + 2
    } else {
      counter <<- counter - 1
    }
  }
  counter <- 20
  state <- NULL
  ws$close()
  while(counter > 0) {
    later::later(close_fn)
    later::run_now(2)
  }

  expect_identical(found, 4)
})
