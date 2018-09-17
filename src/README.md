Notes about compiled sources
============================

## websocketpp

This package makes use of the [websocketpp](https://github.com/zaphoyd/websocketpp) C++ library. It can conflict with another copy of websocketpp that is used in the RStudio IDE, leading to a crash ([#7](https://github.com/rstudio/websocket/issues/7) and [rstudio/rstudio#2838](https://github.com/rstudio/rstudio/issues/2838)). To avoid the crash, the `websocketpp` namespace has been renamed to `ws_websocketpp` in our copy of it.

The stock version of websocketpp [does not compile in minGW](https://github.com/zaphoyd/websocketpp/issues/478) on Windows, because the version of libstdc++ does not define `std::errc::error_canceled`. This is apparently due to [a bug](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=68307) in GNU libstdc++ which is fixed in later versions of minGW. To make it compile, we used a workaround borrowed from this [pull request](https://github.com/zaphoyd/websocketpp/pull/479) on websocketpp which has not been merged because the project seems to be no longer maintained.

To update websocketpp, use the script at src/lib/update.sh. It will perform the namespace renaming but will NOT apply the `std::errc::error_canceled` patch described in the above paragraph; you must perform that change manually before committing.

