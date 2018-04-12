# websocketClient

This is an R WebSocket client library backed by the [websocketpp](https://github.com/zaphoyd/websocketpp) C++ library.

## Development setup

Currently we do local development by running a simple httpuv-backed WebSocket server written in R that lives at `tmp/websocketServer.R`.

To run it, you need to install the Github version of `httpuv`.

### Running testing server on macOS

> Note: If you want httpuv to build faster, you can create a file at ~/.Renviron with the following content: `MAKEFLAGS=-j4`

Next run `remotes::install_github("rstudio/httpuv")`

Then run `R -e 'source("tmp/websocketServer.R");httpuv::service(Inf)'`

## Usage examples

TODO
