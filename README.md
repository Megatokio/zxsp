# zxsp

zxsp – ZX Spectrum emulator for macos (Linux to come)

To get the compiled application go to the [zasm homepage](https://k1.spdns.de/Develop/Projects/zxsp/Distributions). Reportedly it runs quite well on any Mac since 2009.

To compile this project you currently need an old Mac, macos 10.12 SIERRA is best (because that's what i use), Qt 5.xx (Qt 5.09..5.13 are tested), Qt Creator (4.10.2 is the latest version running on macos 10.12) and my [Libraries](https://github.com/Megatokio/Libraries.git) project (possibly branch WORK or something) and the [zasm](https://github.com/Megatokio/zasm.git) Z80 assembler project side-by-side with this project in the same folder and a version of my script interpreter [vipsi](https://k1.spdns.de/Develop/Projects/vipsi/Distributions/) for adding the macos stuff to the app bundle. I'm working on that.

Development on Linux works with up-to-date software, but can't create the macos executable and the Linux version is still in progress.

## Current state of work

The code base was messed up around 2014 or so, when i certainly made backups but did not yet use git. Though i added a lot of new features i didn't release a new version because of a number of bugs that i couldn't fix in time.

Currently i'm in the progress of isolating various parts of the source to make them better maintainable. Once the Model and the View and the OS stuff are properly isolated i'll work on the Linux port.




