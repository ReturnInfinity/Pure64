# Pure64 boot process
Pure64 uses two different first stage loaders depending on the firmware of the system. Older systems will be using BIOS while newer systems will be using UEFI.

## BIOS boot (`bios.asm`):
* BIOS loads the first 512-byte sector from the boot disk to `0x7C00`
* The boot code pulls some data about the system (memory map), sets the video mode, and loads the second stage loader to `0x8000`. 
* The boot code transitions from 16-bit real mode to 32-bit protected mode and then jumps to the second stage.
* The second stage immediately transitions from 32-bit protected mode to a minimal 64-bit long mode. It then follows unified boot path.

## UEFI boot (`uefi.asm`):
* UEFI loads `\EFI\BOOT\BOOTX64.EFI` from the FAT32 partition of the boot disk.
* The boot code sets the video mode, copies the second stage loader to `0x8000`, gets the system memory map, and then jumps to the second stage loader.
* The second stage then follows the unified boot process as the system is already in 64-bit mode.

## Unified boot with the second stage (`pure64.asm`):
* Once it 64-bit mode it configures the PIC, PIT, serial port, sets up the proper 64-bit environment.
* Patches the second stage loader so that the AP's can start at `0x8000` as well.
* The software processes the system memory map and creates a list of memory that is usable.
* All available memory above 4MiB is mapped to `0xFFFF800000000000`
* Exception gates are built at `0x0`
* ACPI tables are parsed
* BSP and HPET are configured
* PIC is activated
* SMP is activated
* An information table is built at `0x5000`
* The (up to) 26 KiB of binary data directly after Pure64 is copied to `0x100000`
* Jumps to `0x100000` to start the kernel
