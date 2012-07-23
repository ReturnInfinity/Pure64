# Pure64 -- a 64-bit loader written in Assembly for x86-64 systems #
Copyright (C) 2007-2012 Return Infinity -- see LICENSE.TXT

Pure64 is a 64-bit software loader initially created for BareMetal OS. The
loader gets the computer into a full 64-bit state with no legacy compatibility
layers and also enables all available CPU Cores in the computer. If you need a
quick way to boot a 64-bit AMD/Intel based computer that will enable all
available processors and load your software then Pure64 is ideal. Pure64 keeps
an information table in memory that stores important details about the
computer (Amount of RAM and memory layout, number of CPU cores and their APIC
IDs, etc). The Pure64 loader has been released separately so others can use it
in their own software projects.

See LICENSE.TXT for redistribution/modification rights, and CREDITS.TXT for a
list of people involved.

Ian Seyler (ian.seyler@returninfinity.com)


## Building

To build all required Pure64 binaries, including MBRs, use the makefile:

    make pure64.sys
    make FAT16.mbr
    make BMFS.mbr

Alternatively, a raw disk image can be produced using:

    make img

If no other options are specified, Pure64 will be built with FAT16 support via
ATA PIO. The Pure64 second stage will be called "PURE64.SYS", and the kernel
binary (not copied onto the disk image by default) will be called
"KERNEL64.SYS".

A PXE binary can be created using:

    make pxe KERNELPATH=/path/to/your/kernel64.sys

Command-line options can be used to build Pure64 with support for BMFS volums,
as well as AHCI drives. The following options are supported:

* `FS`: either `FAT16` (default) or `BMFS`; builds Pure64 with support for the
  named filesystem only.
* `HDD`: either `PIO` (default) or `AHCI`; builds Pure64 with support for the
  named hard disk interface only.
* `KERNELPATH`: a path to a kernel file, which will be appended to the Pure64
  second stage, moved to 0x100000 and executed once Pure64 finishes loading.
  Required for `make pxe`, and optional for other modes. If present, Pure64
  will not bother looking for a kernel file on disk.
* `LOADER`: the name of the Pure64 second stage file on FAT16 volumes;
  defaults to "PURE64.SYS".
* `KERNEL`: the name of the kernel file Pure64 loads from FAT16 volumes;
  defaults to "KERNEL64.SYS".
* `IMAGESIZE`: the size of the disk image created by `make img`; defaults to
  16MiB.

The Pure64 second stage ("PURE64.SYS") can be built directly using nasm:

    nasm pure64.asm -o pure64.sys


## Notes

Pure64 requires at least nasm v2; the version included in the OS X 10.7
Developer Tools is not recent enough.

FAT16 disk image generation has not been tested on any platform other than
OS X.

To use an alternate nasm binary, run:

    NASM=/path/to/other/nasm make -e <target>

To create FAT16 disk images on OS X, run:

    DISKUTIL=hdiutil make -e img


## Compatibility

All modes have been tested using VirtualBox 4.1.18 (you must build using PIO
for IDE drives, and AHCI for SATA drives).

PIO modes have been tested using VMware Fusion 4.1.3.


