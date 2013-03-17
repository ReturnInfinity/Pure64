# Pure64 - v0.6.0 Manual #

Pure64 must be loaded to memory address `0x0000:0x8000`

Pure64 is sector dependant when booting via a hard disk.


## Hard disk boot ##

`bmfs_mbr.asm` in the bootsectors folder shows how to boot via a BMFS formatted drive.

## Network boot ##