# Pure64 - a software loader for x86-64 systems

Pure64 is a software loader that was initially created for BareMetal OS.
The loader sets the computer into a full 64-bit state with no legacy compatibility layers and also enables all available CPU Cores in the computer.
Pure64 keeps an information table in memory that stores important details about the computer (Amount of RAM and memory layout, number of CPU cores and their APIC IDs, etc).
The Pure64 loader has been released separately so others can use it in their own software projects.


## Prerequisites

The scripts in this repo depend on a Debian-based Linux system like [Ubuntu](https://www.ubuntu.com/download/desktop) or [Elementary](https://elementary.io). macOS is also supported if you are using [Homebrew](https://brew.sh).

- [NASM](https://nasm.us) - Assembly compiler to build the loader and boot sectors.

In Linux this can be completed with the following command:

	sudo apt install nasm


## Building the source code

	./build.sh


// EOF
