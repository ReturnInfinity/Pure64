riscv64 Pure64
==============

This code contains the riscv64 bindings for Pure64.

### Building

To build it, you'll need binutils and gcc cross compiled to target riscv64-none-elf.

For binutils (in binutil's project directory):

```
mkdir riscv64-build
cd riscv64-build
../configure --prefix=/opt/cross --target=riscv64-none-elf --with-sysroot --disable-nls --disable-werror
make
sudo make install
```

For GCC (in GCC's project directory):

```
mkdir riscv64-build
cd riscv64-build
../configure --prefix=/opt/cross --target riscv64-none-elf --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
sudo make install-gcc
sudo make install-target-libgcc
```

Be sure to add `/opt/cross/bin` to your `PATH` variable, if it's not already there.

### Testing

To test it, you'll need the riscv port of QEMU (you can find that [here](https://github.com/riscv/riscv-qemu)).
