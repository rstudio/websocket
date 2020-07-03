#!/bin/bash

COMMIT=0.8.2

set -e

rm -rf websocketpp websocketpp_repo
git clone --depth=1 -b "$COMMIT" https://github.com/zaphoyd/websocketpp websocketpp_repo
(cd websocketpp_repo && git checkout "$COMMIT")
rm websocketpp_repo/websocketpp/CMakeLists.txt
export LC_CTYPE=C
export LANG=C
cd websocketpp_repo/websocketpp
find . -path ./.git -prune -o -type f -print0 | xargs -0 sed -i "" -e 's/websocketpp::/ws_websocketpp::/g'
find . -path ./.git -prune -o -type f -print0 | xargs -0 sed -i "" -e 's/namespace websocketpp/namespace ws_websocketpp/g'
find . -path ./.git -prune -o -type f -print0 | xargs -0 sed -i "" -e 's/&std::cout/(std::ostream*)\&WrappedOstream::cout/g'
find . -path ./.git -prune -o -type f -print0 | xargs -0 sed -i "" -e 's/&std::cerr/(std::ostream*)\&WrappedOstream::cerr/g'
cd ../..
mv websocketpp_repo/websocketpp .
rm -rf websocketpp_repo

echo "IMPORTANT NOTE: Apply this patch manually:" >&2
echo "https://github.com/rstudio/websocket/commit/063ca452c7639b952dfd4981602436d43305457a" >&2
