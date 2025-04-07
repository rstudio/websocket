## R CMD check results

There were no ERRORs or WARNINGs.

There were 2 NOTEs:

* checking installed package size ... NOTE
  installed size is  19.8Mb
  sub-directories of 1Mb or more:
    libs   19.6Mb

The websocketpp C++ library is several megabytes in size, so this is unavoidable.

* GNU make is a SystemRequirements.

GNU syntax += is used in Makevars.in to append to the PKG_LIBS variable.

## revdepcheck results

We checked 8 reverse dependencies, comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 0 new problems
 * We failed to check 0 packages

