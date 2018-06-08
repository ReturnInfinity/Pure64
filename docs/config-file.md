Configuration Files
===================

Configuration files are used to describe the layout of the disk image.
They provide a means to customize the layout so that it is suitable to the application.

### Comments

Comments are started with a hash tag '#' and end at the first newline following the hash tag.
While comments may not be needed in most cases, they are used in the examples to explain the
meaning of various options.

### Variables

There are several variables used for creating the disk image.
There are not required to be in any order, but some variables do require
the presence of other variables.

The `arch` option specifies the architecture for boot process.
Currently, only `x86_64` is supported. This variable should always
be present in a config file, for when other architectures become supported.
Here's what it might look like in the file.

    arch: x86_64

For Pure64 to find the bootloader files, it looks in a place called the resource path.
The resource path can be set by either:

  - The compile time default, `/opt/return-infinity/share/pure64/resources`
  - The environment variable, `PURE64_RESOURCE_PATH`
  - The `resource_path` key in the config file.

Use the `resource_path` key if the resources are located somewhere in the project tree.
For example:

    resource_path: "./src/bootloaders/pure64/resources"

The `disk_size` option specifies the minimum size of the disk. Most standard
suffixes, such as `M`, `MiB`, and `MB` are supported. Depending on the other
options, the disk size may be larger than what this variable indicates, but it
will not be smaller. Here's some examples of what it might look like in the file.

    disk_size: 128MiB
    disk_size: 2K
    disk_size: 1024B

The `partition_scheme` option specifies what partitioning system that the
disk will use. If no partition scheme is desired, this field may be left out
or may be set to `none`. The only partitioning scheme currently supported is
the `gpt` partitioning scheme.

    partition_scheme: gpt
    partition_scheme: none

The `fs_loader` option specifies whether or not to include the file system loader.
If it is set to true, the bootloader expects to find a kernel executable at the
`/boot/kernel` path in the Pure64 file system. If this variable is set to false,
then the `kernel_path` variable should be set to the path of the kernel on the host
file system.
