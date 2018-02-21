# websocketClient

This is an R WebSocket client library backed by the [easywsclient](https://github.com/dhbaird/easywsclient) C++ library.

## Development setup

Currently we do local development by running a simple httpuv-backed WebSocket server written in R that lives at `tmp/websocketServer.R`.

To run it, you need to install the Github version of `httpuv`.

### Running testing server on macOS

> Note: If you want httpuv to build faster, you can create a file at ~/.Renviron with the following content: `MAKEFLAGS=-j4`

First you'll need [Homebrew](https://brew.sh). Then install the following Homebrew packages:

```
brew install automake libtool
```

Next run `remotes::install_github("rstudio/httpuv")`

Then at an `R` prompt source the server with `source("tmp/websocketServer.R")`

## Usage examples

TODO
