# zxsp

_ZX Spectrum emulator for Mac OSX_

The latest compiled version and older versions can be downloaded at [zxsp homepage](https://k1.spdns.de/Develop/Projects/zxsp/Distributions).

_zxsp_ runs on Intel Macs with Mac OS X 10.6 "Snow Leopard" and above. Reportedly it also runs well on ARM based Macs with macos 13.1 "Ventura". Latest report is for macos 15.6.1 "Sequoia".
So it currently should run on any Mac since 2009.

To compile this project you currently need an old Mac, Mac OS X 10.12 "Sierra" is best, Qt 5.09 to 5.13, Qt Creator 4.10.2, which is the latest version running on Mac OS X 10.12, and possibly a version of my script interpreter [vipsi](https://k1.spdns.de/Develop/Projects/vipsi/Distributions/) for adding the macos stuff to the app bundle and run the Qt utility _macdeployqt_ on it. But you can also just copy the libraries and resources from an existing _zxsp_ installation. Make shure to checkout the _zxsp_ git archive with submodules, because it contains _zasm_ which in return contains the _Libraries_.

## Current state of work

The code base was messed up around 2014 or so, when i certainly made backups but did not yet use git. Though i added a lot of new features i didn't release a new version because of a number of bugs that i couldn't fix in time.

Currently i'm in the progress of isolating various parts of the source to make them better maintainable. Meanwhile i mostly only do bug fixes.




