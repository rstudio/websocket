#!/bin/bash

COMMIT=0.8.1

set -e

rm -rf websocketpp websocketpp_repo
git clone https://github.com/zaphoyd/websocketpp websocketpp_repo
(cd websocketpp_repo && git checkout "$COMMIT")
rm websocketpp_repo/websocketpp/CMakeLists.txt
export LC_CTYPE=C
export LANG=C
cd websocketpp_repo/websocketpp
find . -path ./.git -prune -o -type f -print0 | xargs -0 sed -i "" -e 's/websocketpp::/ws_websocketpp::/g'
find . -path ./.git -prune -o -type f -print0 | xargs -0 sed -i "" -e 's/namespace websocketpp/namespace ws_websocketpp/g'
find . -path ./.git -prune -o -type f -print0 | xargs -0 sed -i "" -e 's/&std::cout/(std::ostream*)\&Rcpp::Rcout/g'
find . -path ./.git -prune -o -type f -print0 | xargs -0 sed -i "" -e 's/&std::cerr/(std::ostream*)\&Rcpp::Rcerr/g'
cd ../..
mv websocketpp_repo/websocketpp .
rm -rf websocketpp_repo
