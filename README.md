# Pure64 -- an OS/software loader for x86-64 systems

Copyright (C) 2008-2020 Return Infinity -- see LICENSE.TXT

Pure64 is a software loader that was initially created for BareMetal OS.
The loader sets the computer into a full 64-bit state with no legacy compatibility layers and also enables all available CPU Cores in the computer.
Pure64 keeps an information table in memory that stores important details about the computer (Amount of RAM and memory layout, number of CPU cores and their APIC IDs, etc).
The Pure64 loader has been released separately so others can use it in their own software projects.

See LICENSE.TXT for redistribution/modification rights, and CREDITS.TXT for a list of people involved.

Ian Seyler (ian.seyler@returninfinity.com)

## Building

The only requirement for building Pure64 is [NASM](http://www.nasm.us/) (The Netwide Assembler) and GCC.
In Linux you can download it from your distributions repository(`apt-get install nasm gcc`).
If you are using Windows or macOS you can grab pre-compiled binaries [here](http://www.nasm.us/pub/nasm/releasebuilds/2.14.02/) in the `macosx` and `win32` directories, respectively.
For GCC, you can use MinGW [here](https://sourceforge.net/projects/mingw/files/).

Build scripts are included for macOS/Linux and Windows systems.

macOS/Linux: `./build.sh`

Windows: `build.bat`

## Notes

Building Pure64 from source requires NASM v2.10 or higher; the version included in the macOS 10.12 Developer Tools is not recent enough. - *Seriously, Apple? NASM v0.98 is from 2007!!*

If you use [MacPorts](http://www.macports.org), you can install NASM v2.10+ by executing: `sudo port install nasm`

If you use [Homebrew](https://brew.sh), you can install NASM 2.10+ by executing: `brew install nasm`

// EOF
