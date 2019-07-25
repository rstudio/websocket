context("wsOptions")

test_that("wsOptions can list", {
  expect_identical(wsOptions(), options)
})

test_that("wsOptions fails with bad input", {
  expect_error(wsOptions(c(named="vector")), "wsOptions accepts named options")
  expect_error(wsOptions(letters[1:2]), "wsOptions accepts named options")
  expect_error(wsOptions(list(IDONTEXIST="abc", MENEITHER=123)), "wsOptions accepts named options")
  expect_error(wsOptions(IDONTEXIST="abc", MENEITHER=123), "Unrecognized options provided: IDONTEXIST, MENEITHER")
})
