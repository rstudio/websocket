## R CMD check results

There were no ERRORs or WARNINGs.

There were 2 NOTEs:

* checking installed package size ... NOTE
  installed size is 14.1Mb
  sub-directories of 1Mb or more:
    libs  13.9Mb

The websocketpp C++ library is several megabytes in size, so this is unavoidable.

* GNU make is a SystemRequirements.

GNU syntax += is used in Makevars.in to append to the PKG_LIBS variable.
