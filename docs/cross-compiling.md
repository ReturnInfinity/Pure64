Cross Compilation
=================

To cross compile the utility, use the `CROSS_COMPILE` variable (and the `EXE` variable, if needed).

For example, cross compiling the utility for Windows is done like this:

    make CROSS_COMPILE=x86_64-w64-mingw32- EXE=.exe

And to install it, this is done:

    make CROSS_COMPILE=x86_64-w64-mingw32- EXE=.exe install PREFIX=/some/install/path

The bootloader is setup to be cross compiled by default.
There are existing targets in the top level Makefile to assist with this.
For xample, to cross compile for x86_64:

    make clean-pure64-x86_64
    make all-pure64-x86_64
    make install-pure64-x86_64
