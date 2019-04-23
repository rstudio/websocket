# 2019-04-23

This is a resubmission after changing the Title field in DESCRIPTION. It no longer begins with the name of the package.

Thank you,
Alan

# 2019-03-25

This is a resubmission after amending the Description per the feedback below. I
did not remove the LICENSE file because websocketpp is BSD-licensed, and I
believe its license must be included in order to comply with its terms.

Thank you,
Alan

# 2019-03-24

Thanks,

Please write package names, software names and API names in single 
quotes (e.g. 'R') in the Description field.

The LICENSE file is only needed if you have additional restrictions to 
the GPL-2 which you have not? In that case omit the file and its 
reference in the DESCRIPTION file.

Please fix and resubmit.

Best,
Martina Schmirl

# 2019-03-19

Thanks, can you please explain what "WebSocket" is? Perhaps add an URL
and point to it in the form <http....>?

Best,
Uwe Ligges

# 2019-03-18

This is a new submission.

## Test environments

* local Windows 10 install, R 3.5.3
* Ubuntu 14.04 (on travis-ci), R 3.5.3
* Ubuntu 14.04 (on travis-ci), R-devel

## R CMD check results

There were no ERRORs or WARNINGs.

There were 2 NOTEs:

* installed size is 13.7Mb
  sub-directories of 1Mb or more:
  libs  13.5Mb
  
The websocketpp C++ library object files exceed 1Mb.
    
* GNU make is a SystemRequirements.

GNU syntax += is used in Makevars.in to append to the PKG_LIBS variable.
