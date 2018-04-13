Set up
------

Because this repo is private, you need to supply an `auth_token` argument to the `install_github` function in order to download this package from R. Before running the code below you must first create a personal access token with "repo" permissions at this URL: [https://github.com/settings/tokens](https://github.com/settings/tokens)

``` r
install.packages("remotes")
remotes::install_github("rstudio/websocket", auth_token = "YOUR_GITHUB_PERSONAL_ACCESS_TOKEN")
```

Creating a websocket
--------------------

First, create a websocket client by running:

``` r
# Create a websocket using the websocket.org test server
ws <- websocket::WebsocketClient$new("ws://echo.websocket.org/",
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

For each message `msg` you send, the following should be printed to the
R console: “Client got msg: `msg`”.

Closing the websocket
---------------------

Finally, when you’re done, close the websocket:

``` r
ws$close()
```

When you do this, the following should be printed to the R console:
“Client disconnected”
