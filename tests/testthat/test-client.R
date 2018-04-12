context("client")

test_that("Basic websocket communication", {
  state <- NULL
  last <- NULL

  ws <- WebsocketClient$new("ws://echo.websocket.org/",
    onMessage = function(msg) last  <<- msg,
    onOpen    = function()    state <<- "open",
    onClose   = function()    state <<- "closed",
    onFail    = function()    state <<- "failed"
  )

  # Make sure the internal state gets set, and the onOpen function gets called.
  expect_identical(ws$getState(), "OPEN")
  expect_identical(state, "open")

  ws$send("hello")
  expect_identical(last, "hello")

  ws$send(charToRaw("hello"))
  expect_identical(last, charToRaw("hello"))

  ws$close()
  expect_identical(ws$getState(), "CLOSED")
  expect_identical(state, "closed")
})


test_that("Basic ssl websocket communication", {
  state <- NULL
  last <- NULL

  ws <- WebsocketClient$new("wss://echo.websocket.org/",
    onMessage = function(msg) last  <<- msg,
    onOpen    = function()    state <<- "open",
    onClose   = function()    state <<- "closed",
    onFail    = function()    state <<- "failed"
  )

  expect_identical(ws$getState(), "OPEN")
  expect_identical(state, "open")

  ws$send("hello")
  expect_identical(last, "hello")

  ws$send(charToRaw("hello"))
  expect_identical(last, charToRaw("hello"))

  ws$close()
  expect_identical(ws$getState(), "CLOSED")
  expect_identical(state, "closed")
})
