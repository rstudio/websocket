## Comments

#### 2021-08-18

Bug fixes.

Thank you,
Winston


## Test environments and R CMD check results

* GitHub Actions - https://github.com/rstudio/websocket/pull/86/checks
  * macOS
    * devel, release
  * windows
    * release, 3.6
  * ubuntu20
    * devel, release, oldrel/1, oldrel/2, oldrel/3, oldrel/4
* devtools::
  * check_win_devel()
  * check_win_release()
  * check_win_oldrelease()


## R CMD check results

There were no ERRORs or WARNINGs.

There were 2 NOTEs:

* checking installed package size ... NOTE
  installed size is  9.6Mb
  sub-directories of 1Mb or more:
    libs   9.5Mb

The websocketpp C++ library is several megabytes in size, so this is unavoidable.

* GNU make is a SystemRequirements.

GNU syntax += is used in Makevars.in to append to the PKG_LIBS variable.

## revdepcheck results

We checked 6 reverse dependencies, comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 0 new problems
 * We failed to check 0 packages
