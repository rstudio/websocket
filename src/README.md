Notes about compiled sources
============================

## websocketpp

This package makes use of the [websocketpp](https://github.com/zaphoyd/websocketpp) C++ library. It can conflict with another copy of websocketpp that is used in the RStudio IDE, leading to a crash ([#7](https://github.com/rstudio/websocket/issues/7) and [rstudio/rstudio#2838](https://github.com/rstudio/rstudio/issues/2838)). To avoid the crash, the `websocketpp` namespace has been renamed to `ws_websocketpp` in our copy of it.
