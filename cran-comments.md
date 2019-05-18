# 2019-05-18

I am resubmitting after realizing that in order to comply with the terms of
WebSocket++ and constituent libraries -- even as they are compatible with
GPL-2 -- I must include their licenses in the source code.

In the prior submission I removed LICENSE per feedback; in this submission
I brought it back as the natural place to include library licenses.

Please consider my previous submission invalid. I apologize
for the confusion.

Thank you,
Alan

# 2019-05-17

This is a resubmission after clarifying that websocket is licensed GPL-2 (by
removing LICENSE) and adding library authors to DESCRIPTION, per feedback.

websocket includes the WebSocket++ library, which itself includes several other
libraries with a variety of licenses: https://github.com/zaphoyd/websocketpp/blob/master/COPYING

WebSocket++ and the libraries it includes are all distributed under licenses
that are compatible with GPL-2 by by
https://www.gnu.org/licenses/license-list.en.html#GPLCompatibleLicenses.

2 use zlib, 1 uses MIT, 2 use Modified BSD.

Consequently, it is not necessary to obtain permission from these library
authors to redistribute their software under the terms of GPL-2 as part of the
websocket package.

Thank you,
Alan

# 2019-05-17

On 17.05.2019 16:51, Uwe Ligges wrote:
> Thanks,
>
> this is difficult.
>
> you license the package under GPL-2. Are you sure you can relicense all
> included parts under GPL-2? Please tell us if that is true and if
> needed, wehther you have permissions of the copyright holders.

And note that the license file is not valid as a package must be
licensed under one license or another, but not a combination of licenses.

Best,
Uwe Ligges

# 2019-05-17

Thanks,

this is difficult.

you license the package under GPL-2. Are you sure you can relicense all
included parts under GPL-2? Please tell us if that is true and if
needed, wehther you have permissions of the copyright holders.

We also see that you dfailed to mention all authors, contributors and
copright holders in the Authors@R field. As an examples, you have files
containing

  * Copyright (c) 2014, Peter Thorson. All rights reserved.

and that name does not appear in your DESCRIPTION file at all.

Please really follow the CRAN policies and carefully check the first of
our policy you confirmed:
"The ownership of copyright and intellectual property rights of all
components of the package must be clear and unambiguous (including from
the authors specification in the DESCRIPTION file). Where code is copied
(or derived) from the work of others (including from R itself), care
must be taken that any copyright/license statements are preserved and
authorship is not misrepresented.

Preferably, an ‘Authors@R’ would be used with ‘ctb’ roles for the
authors of such code. Alternatively, the ‘Author’ field should list
these authors as contributors.

Where copyrights are held by an entity other than the package authors,
this should preferably be indicated via ‘cph’ roles in the ‘Authors@R’
field, or using a ‘Copyright’ field (if necessary referring to an
inst/COPYRIGHTS file).

Trademarks must be respected."

Please fix and resubmit.

Best,
Uwe Ligges

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
