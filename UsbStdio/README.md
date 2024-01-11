# UsbStdio #

Test of USB Stdio input

Note that the file called `main_NOT_WORKING.cpp' shows an issue with reading the character in a callback, which isn't working (see issue https://github.com/raspberrypi/pico-sdk/issues/1603). It's been worked around using an async worker that you'll find in `main.cpp`.
