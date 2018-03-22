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

The `disk_size` option specifies the minimum size of the disk. Most standard
suffixes, such as `M`, `MiB`, and `MB` are supported. Depending on the other
options, the disk size may be larger than what this variable indicates, but it
will not be smaller.

The `partition_scheme` option specifies what partitioning system that the
disk will use. If no partition scheme is desired, this field may be left out
or may be set to `none`. The only partitioning scheme currently supported is
the `gpt` partitioning scheme.

The `stage_three` option specifies what code should be run once the CPU is
fully initialized. Either the kernel can be started after this stage, by
specifying `kernel`, or the file system loader can be started by specifying `loader`.

If `stage_three` is set to `kernel`, then the `kernel_path` variable should also
be set. The `kernel_path` variable tells Pure64 where to find the kernel file.
