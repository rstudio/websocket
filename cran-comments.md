## R CMD check results

There were no ERRORs or WARNINGs.

There were 2 NOTEs:

* possible bashism in configure line 34 ($(OS|MACH)TYPE):
    case "$OSTYPE" in "darwin"*)
  possible bashism in configure line 79 ($(OS|MACH)TYPE):
  case "$OSTYPE" in

The use of `$OSTYPE` is the same as is used in the openssl package. I don't currently know of another way to achieve the same thing.

* GNU make is a SystemRequirements.

GNU syntax += is used in Makevars.in to append to the PKG_LIBS variable.


## Regarding Package Check Results

On https://cloud.r-project.org/web/checks/check_results_websocket.html, the
`r-patched-solaris-x86` platforms are listed with
status ERROR.

### `r-patched-solaris-x86`

We have used a Solaris VM, but haven't been able to reproduce the error at https://www.r-project.org/nosvn/R.check/r-patched-solaris-x86/websocket-00check.html when running the tests manually, or with `R CMD check`. I believe it may be an intermittent error that is happening due to network timeouts on the Solaris build machine.
