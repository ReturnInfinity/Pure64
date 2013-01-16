# Pure64 -- The BareMetal OS kernel loader #
Copyright (C) 2007-2013 Return Infinity -- see LICENSE.TXT

Pure64 is a 64-bit software loader for BareMetal OS. The loader gets the computer into a full 64-bit state with no legacy compatibility layers and also enables all available CPU Cores in the computer. If you need a quick way to boot a 64-bit AMD/Intel based computer that will enable all available processors and load your software then Pure64 is ideal. Pure64 keeps an information table in memory that stores important details about the computer (Amount of RAM and memory layout, number of CPU cores and their APIC IDs, etc). The Pure64 loader has been released separately so others can use it in their own software projects.

See LICENSE.TXT for redistribution/modification rights, and CREDITS.TXT for a list of people involved.

Ian Seyler (ian.seyler@returninfinity.com)


## Building

The Pure64 second stage ("PURE64.SYS") can be built directly using nasm:

    nasm pure64.asm -o pure64.sys


## Notes

Pure64 requires at least nasm v2; the version included in the OS X 10.7 Developer Tools is not recent enough.
