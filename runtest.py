#!/usr/bin/python3

# A brief introduction to the OS/90 build system.
#
# Drivers and the kernel both use makefiles to build.
# A single "make" command is issued.
#
# The kernel has a static named binary named OS90.DAT.
# Driver can be named anything. They go in the `drivers` directory
# and must have a makefile. in the driver root directory.
#
# The order in which drivers are loaded is defined by a .(number)
# suffix to the directory name.
#
# For example, PCI.1 or ATA.0.
#
# If the number is the same as another, the group will occur together
# in any oder. If there is no number, any order is possible.
#

#
# Packages are another concept. This is used to add certain userspace
# utilities. This is similar to a package manager but only for building.
# OS/90 itself does not have a package manager.
#