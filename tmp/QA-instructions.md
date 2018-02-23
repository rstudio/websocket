Set up
------

``` r
install.packages("remotes")
remotes::install_github("rstudio/websocketClient")
```

Creating a websocket
--------------------

First, create a websocket client by running:

``` r
# Create a websocket using the websocket.org test server
ws <- websocketClient::WebsocketClient$new("ws://echo.websocket.org/",
  onMessage = function(msg) {
    cat("Client got msg: ", msg, "\n")
  },
  onDisconnected = function() {
    cat("Client disconnected\n")
  }
)
```

For now, this displays an error with the red message:

    easywsclient: connecting: host=echo.websocket.org port=80 path=/
    Connected to: ws://echo.websocket.org/

This is an unfortunate side-effect from the underlying c++ library we’re
using, but it doesn’t affect the functionality.

Sending and receiving messages
------------------------------

Then, you can send an arbitrary number of messages like so:

``` r
ws$send("hello")
```

For each message `msg` you send, you’ll receive another message: “Client
got msg: `msg`”.

Closing the websocket
---------------------

Finally, when you’re done, close the websocket:

``` r
ws$close()
```
