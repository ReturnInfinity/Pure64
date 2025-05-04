# Pure64 Manual

This manual has the purpose of helping software developers understand how they should be using Pure64 to load their kernel.


## Prerequisites

This manual assumes you have some understanding of a typical boot process.

You'll have to have NASM installed to build the software.


## Building

In the top directory of Pure64, just run the following:

```
./build.sh
```


## System Requirements

A computer with at least one 64-bit Intel or AMD CPU (or anything else that uses the x86-64 architecture)

At least 2 MiB of RAM

The ability to boot via a hard drive, USB stick, or the network


## Writing the Kernel with NASM

Here's a minimal kernel, written for NASM, that you could use with Pure64.
Once it's loaded, it enters an infinite loop.
The file can be called `kernel.asm`.

```
BITS 64
ORG 0x100000

start:
	jmp start
```

The `ORG` statement tells NASM that the code should be made to run at the address, `0x100000`.

Assemble it like this:

```
nasm kernel.asm -o kernel.bin
```

## Writing a Kernel with GCC

Here's a similar example written in C with GCC.
The file can be called `kernel.c`.

```
void _start(void) {

	for (;;) {

	}
}
```

The kernel needs a linker script to be loaded to 1 MiB, replacing NASMs `ORG` instruction.
The file can be called `kernel.ld`.

```
OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386:x86-64")

SECTIONS
{
	. = 0x100000;
	.text : {
		*(.text)
	}
	.data : {
		*(.data)
	}
	.rodata : {
		*(.rodata)
	}
	.bss : {
		*(.bss)
	}
}

```

Compile is like this:

```
gcc -c kernel.c -o kernel.o -mno-red-zone -fno-stack-protector -fomit-frame-pointer
ld -T kernel.ld -o kernel.bin kernel.o
```

The flags added to the first command are there to help GCC produce could that will run in kernel space.
The second command simply takes kernel.o and orders it as the linker script tells it to.

## Contributors note

The `_start` symbol must always appear first within flat binaries as Pure64 will call the start of the file so it must contain executable code. Function definitions (such as inline ones) in header files could interfere with the placement of the `_start` symbol. The best solution is to put the entry point in a separate file that calls the main function. Such a file could be called `start.c`.
```
extern int main(void);

void _start(void)
{
	main();
}
```
This file would **always** have to be linked in front of everything else. For the above example that would mean the linker command above would have to become:
```
ld -T kernel.ld -o kernel.bin start.o kernel.o
```

## Creating a bootable image

After creating a kernel this is a possible routine to create a bootable image.
The commands require Pure64 to be build and `pure64.sys` and `mbr.sys` to be in the same directory 
as your kernel with the name `kernel.bin`

```
dd if=/dev/zero of=disk.img count=128 bs=1048576
cat pure64.sys kernel.bin > software.sys

dd if=mbr.sys of=disk.img conv=notrunc
dd if=software.sys of=disk.img bs=512 seek=16 conv=notrunc
```

After creating a bootable image it can be tested using qemu:
`qemu-system-x86_64 -drive format=raw,file=disk.img`

## Memory Map

This memory map shows how physical memory looks after Pure64 is finished.

<table border="1" cellpadding="2" cellspacing="0">
<tr><th>Start Address</th><th>End Address</th><th>Size</th><th>Description</th></tr>
<tr><td>0x0000000000000000</td><td>0x0000000000000FFF</td><td>4 KiB</td><td>IDT - 256 descriptors (each descriptor is 16 bytes)</td></tr>
<tr><td>0x0000000000001000</td><td>0x0000000000001FFF</td><td>4 KiB</td><td>GDT - 256 descriptors (each descriptor is 16 bytes)</td></tr>
<tr><td>0x0000000000002000</td><td>0x0000000000002FFF</td><td>4 KiB</td><td>PML4 - 512 entries, first entry points to PDP at 0x3000</td></tr>
<tr><td>0x0000000000003000</td><td>0x0000000000003FFF</td><td>4 KiB</td><td>PDP Low - 512 entries</td></tr>
<tr><td>0x0000000000004000</td><td>0x0000000000004FFF</td><td>4 KiB</td><td>PDP High - 512 entries</td></tr>
<tr><td>0x0000000000005000</td><td>0x0000000000007FFF</td><td>12 KiB</td><td>Pure64 Data</td></tr>
<tr><td>0x0000000000008000</td><td>0x000000000000FFFF</td><td>32 KiB</td><td>Pure64 - After the OS is loaded and running this memory is free again</td></tr>
<tr><td>0x0000000000010000</td><td>0x000000000001FFFF</td><td>64 KiB</td><td>PD Low - Entries are 8 bytes per 2MiB page</td></tr>
<tr><td>0x0000000000020000</td><td>0x000000000005FFFF</td><td>256 KiB</td><td>PD High - Entries are 8 bytes per 2MiB page</td></tr>
<tr><td>0x0000000000060000</td><td>0x000000000009FFFF</td><td>256 KiB</td><td>Free</td></tr>
<tr><td>0x00000000000A0000</td><td>0x00000000000FFFFF</td><td>384 KiB</td><td>BIOS ROM Area</td></tr>
<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>VGA mem at 0xA0000 (128 KiB) Color text starts at 0xB8000</td></tr>
<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>Video BIOS at 0xC0000 (64 KiB)</td></tr>
<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>Motherboard BIOS at F0000 (64 KiB)</td></tr>
<tr><td>0x0000000000100000</td><td>0xFFFFFFFFFFFFFFFF</td><td>1+ MiB</td><td>The software payload is loaded here</td></tr>
</table>

When creating your Operating System or Demo you can use the sections marked free, however it is the safest to use memory above 1 MiB.


## Information Table

Pure64 stores an information table in memory that contains various pieces of data about the computer before it passes control over to the software you want it to load.

The Pure64 information table is located at `0x0000000000005000` and ends at `0x00000000000057FF` (2048 bytes).

<table border="1" cellpadding="2" cellspacing="0">
<tr><th>Memory Address</th><th>Variable Size</th><th>Name</th><th>Description</th></tr>
<tr><td>0x5000</td><td>64-bit</td><td>ACPI</td><td>Address of the ACPI tables</td></tr>
<tr><td>0x5008</td><td>32-bit</td><td>BSP_ID</td><td>APIC ID of the BSP</td></tr>
<tr><td>0x5010</td><td>16-bit</td><td>CPUSPEED</td><td>Speed of the CPUs in MegaHertz (<a href="http://en.wikipedia.org/wiki/Hertz">MHz</a>)</td></tr>
<tr><td>0x5012</td><td>16-bit</td><td>CORES_ACTIVE</td><td>The number of CPU cores that were activated in the system</td></tr>
<tr><td>0x5014</td><td>16-bit</td><td>CORES_DETECT</td><td>The number of CPU cores that were detected in the system</td></tr>
<tr><td>0x5016 - 0x501F</td><td>&nbsp;</td><td>&nbsp;</td><td>For future use</td></tr>
<tr><td>0x5020</td><td>32-bit</td><td>RAMAMOUNT</td><td>Amount of system RAM in Mebibytes (<a href="http://en.wikipedia.org/wiki/Mebibyte">MiB</a>)</td></tr>
<tr><td>0x5022 - 0x502F</td><td>&nbsp;</td><td>&nbsp;</td><td>For future use</td></tr>
<tr><td>0x5030</td><td>8-bit</td><td>IOAPIC_COUNT</td><td>Number of I/O APICs in the system</td></tr>
<tr><td>0x5031</td><td>8-bit</td><td>IOAPIC_INTSOURCE_COUNT</td><td>Number of I/O APIC Interrupt Source Override</td></tr>
<tr><td>0x5032 - 0x503F</td><td>&nbsp;</td><td>&nbsp;</td><td>For future use</td></tr>
<tr><td>0x5040</td><td>64-bit</td><td>HPET Address</td><td>Base memory address for the High Precision Event Timer</td></tr>
<tr><td>0x5048</td><td>32-bit</td><td>HPET Frequency</td><td>Frequency for the High Precision Event Timer</td></tr>
<tr><td>0x504C</td><td>16-bit</td><td>HPET Counter Minumum</td><td>Minimum Counter for the High Precision Event Timer</td></tr>
<tr><td>0x504E</td><td>8-bit</td><td>HPET Counters</td><td>Number of Counter in the High Precision Event Timer</td></tr>
<tr><td>0x504F</td><td>&nbsp;</td><td>&nbsp;</td><td>For future use</td></tr>
<tr><td>0x5060</td><td>64-bit</td><td>LAPIC</td><td>Local APIC address</td></tr>
<tr><td>0x5068 - 0x507F</td><td>&nbsp;</td><td>&nbsp;</td><td>For future use</td></tr>
<tr><td>0x5080</td><td>64-bit</td><td>VIDEO_BASE</td><td>Base memory for video (if graphics mode set)</td></tr>
<tr><td>0x5088</td><td>16-bit</td><td>VIDEO_X</td><td>X resolution</td></tr>
<tr><td>0x508A</td><td>16-bit</td><td>VIDEO_Y</td><td>Y resolution</td></tr>
<tr><td>0x508C</td><td>16-bit</td><td>VIDEO_PPSL</td><td># of pixels per scan line</td></tr>
<tr><td>0x508E</td><td>16-bit</td><td>VIDEO_BPP</td><td># of bits per pixel</td></tr>
<tr><td>0x5090</td><td>16-bit</td><td>PCIE_COUNT</td><td>Number of PCIe buses</td></tr>
<tr><td>0x5092</td><td>16-bit</td><td>IAPC_BOOT_ARCH</td><td>IA-PC Boot Architecture Flags</td></tr>
<tr><td>0x5094 - 0x50DF</td><td>&nbsp;</td><td>&nbsp;</td><td>For future use</td></tr>
<tr><td>0x50E0</td><td>8-bit</td><td>FLAG_1GBPAGE</td><td>1 if 1GB Pages are supported</td></tr>
<tr><td>0x50E1</td><td>8-bit</td><td>FLAG_X2APIC</td><td>1 if X2APIC is supported</td></tr>
<tr><td>0x50E2 - 0x50FF</td><td>&nbsp;</td><td>&nbsp;</td><td>For future use</td></tr>
<tr><td>0x5100 - 0x51FF</td><td>8-bit</td><td>APIC_ID</td><td>APIC ID's for valid CPU cores (based on CORES_ACTIVE)</td></tr>
<tr><td>0x5200 - 0x53FF</td><td>&nbsp;</td><td>&nbsp;</td><td>For future use</td></tr>
<tr><td>0x5400 - 0x55FF</td><td>16 byte entries</td><td>PCIE</td><td>PCIe bus data</td></tr>
<tr><td>0x5600 - 0x56FF</td><td>16 byte entries</td><td>IOAPIC</td><td>I/O APIC addresses (based on IOAPIC_COUNT)</td></tr>
<tr><td>0x5700 - 0x57FF</td><td>8 byte entries</td><td>IOAPIC_INTSOURCE</td><td>I/O APIC Interrupt Source Override Entries (based on IOAPIC_INTSOURCE_COUNT)</td></tr>
</table>

PCIE list format:
<table border="1" cellpadding="2" cellspacing="0">
<tr><th>Offset</th><th>Variable Size</th><th>Name</th><th>Description</th></tr>
<tr><td>0x00</td><td>64-bit</td><td>Base</td><td>The base address of enhanced configuration mechanism</td></tr>
<tr><td>0x08</td><td>16-bit</td><td>Group</td><td>The PCI segment group number</td></tr>
<tr><td>0x0A</td><td>8-bit</td><td>Start Bus</td><td>Start PCI bus number decoded by this host bridge</td></tr>
<tr><td>0x0B</td><td>8-bit</td><td>End Bus</td><td>End PCI bus number decoded by this host bridge</td></tr>
<tr><td>0x0C</td><td>32-bit</td><td>Reserved</td><td>This value should be 0</td></tr>
</table>

IOAPIC list format:
<table border="1" cellpadding="2" cellspacing="0">
<tr><th>Offset</th><th>Variable Size</th><th>Name</th><th>Description</th></tr>
<tr><td>0x00</td><td>32-bit</td><td>I/O APIC ID</td><td>The ID of an I/O APIC</td></tr>
<tr><td>0x00</td><td>32-bit</td><td>I/O APIC Address</td><td>The 32-bit physical address to access this I/O APIC</td></tr>
<tr><td>0x00</td><td>32-bit</td><td>Global System Interrupt Base</td><td>The global system interrupt number where this I/O APIC’s interrupt inputs start</td></tr>
<tr><td>0x00</td><td>32-bit</td><td>Reserved</td><td>This value should be 0</td></tr>
</table>

IOAPIC_INTSOURCE list format:
<table border="1" cellpadding="2" cellspacing="0">
<tr><th>Offset</th><th>Variable Size</th><th>Name</th><th>Description</th></tr>
<tr><td>0x00</td><td>8-bit</td><td>Bus</td><td>0</td></tr>
<tr><td>0x00</td><td>8-bit</td><td>Source</td><td>Bus-relative interrupt source</td></tr>
<tr><td>0x00</td><td>32-bit</td><td>Global System Interrupt</td><td>The Global System Interrupt that this bus-relative interrupt source will signal</td></tr>
<tr><td>0x00</td><td>16-bit</td><td>Flags</td><td>MPS INTI flags</td></tr>
</table>

MPS INTI flags:
<table border="1" cellpadding="2" cellspacing="0">
<tr><th>Flags</th><th>Bit Length</th><th>Bit Offset</th><th>Description</th></tr>
<tr><td>Polarity</td><td>2</td><td>0</td><td>01 Active high, 11 Active low</td></tr>
<tr><td>Trigger Mode</td><td>2</td><td>2</td><td>01 Edge-triggered, 11 Level-triggered</td></tr>
</table>

## System Memory Map

### BIOS

A copy of the BIOS System Memory Map is stored at memory address `0x0000000000006000`. Each BIOS record is 32 bytes in length and the memory map is terminated by a blank record.
<table border="1" cellpadding="2" cellspacing="0">
<tr><th>Variable</th><th>Variable Size</th><th>Description</th></tr>
<tr><td>Base</td><td>64-bit</td><td>Base Address</td></tr>
<tr><td>Length</td><td>64-bit</td><td>Length in bytes</td></tr>
<tr><td>Type</td><td>32-bit</td><td>Type of memory (1 is usable)</td></tr>
<tr><td>ACPI</td><td>32-bit</td><td>See document linked below</td></tr>
</table>
For more information on the BIOS Memory Map: <a href="https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_EAX_=_0xE820">BIOS Specs</a>

### UEFI

A copy of the UEFI System Memory Map is stored at memory address `0x0000000000220000`. Each UEFI record is 48 bytes in length and the memory map is terminated by a blank record.
<table border="1" cellpadding="2" cellspacing="0">
<tr><th>Variable</th><th>Variable Size</th><th>Description</th></tr>
<tr><td>Type</td><td>64-bit</td><td>The type of the memory region</td></tr>
<tr><td>Physical Start</td><td>64-bit</td><td>Physical Address - 4K aligned</td></tr>
<tr><td>Virtual Start</td><td>64-bit</td><td>Virtual Address - 4K aligned</td></tr>
<tr><td>NumberOfPages</td><td>64-bit</td><td>The number of 4K pages in this section</td></tr>
<tr><td>Attribute</td><td>64-bit</td><td>See document linked below</td></tr>
<tr><td>Padding</td><td>64-bit</td><td>Padding</td></tr>
</table>
For more information on the UEFI Memory Map: <a href="https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html#efi-boot-services-getmemorymap">UEFI Specs</a>


## Debugging

Pure64 initializes the serial port `COM1` at 115200bps, 8 data bytes, 1 stop bit, no parity, and no flow control. It will display messages on boot-up.


// EOF
