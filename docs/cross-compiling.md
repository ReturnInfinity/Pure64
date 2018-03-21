Cross Compilation
=================

Both the utility and the boot code that runs on the target computer can be cross compiled.

To cross compile the utility, use the `HOST_CROSS_COMPILE` variable when invoking make.
For example, to cross compile the utility for Windows, you'd do this:

```
make HOST_CROSS_COMPILE=x86_64-w64-mingw32- EXE=.exe
```

To cross compile the utility for aarch64 on a Linux distribution, you'd do this:

```
make HOST_CROSS_COMPILE=aarch64-linux-gnu-
```

The only supported cross compilation target for the boot code is x86_64-none-elf,
since that is currently the only supported architecture.
