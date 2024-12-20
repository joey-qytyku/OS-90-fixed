# Driver Executable Format

Drivers are enhanced flat binaries with added relocation tables so that they can be loaded at arbitrary addresses.

The OS/90 kernel provides no dynamic linking for kernel libraries.

## Basic Structure

## Relocation Table

Relocations with an addend are separated from relocations that do not have one.
