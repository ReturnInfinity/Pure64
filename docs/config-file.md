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

The `disk-size` option specifies the minimum size of the disk. Most standard
suffixes, such as `M`, `MiB`, and `MB` are supported. Depending on the other
options, the disk size may be larger than what this variable indicates, but it
will not be smaller.
