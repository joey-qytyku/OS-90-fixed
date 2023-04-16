# Overview

The userspace specification details the way that programs and the user interface with the system. It is highly recommended that programs follow the userspace protocols. The goal is to ensure that their are no competing standards, and that OS/90 can avoid being fragmented and cluttered, and instead be an integrated experience. OS/90 is intended to be a desktop operating system for only one user.

# INIT program

INIT is the program that initializes the system. It must be 32-bit.

#

# Directory Structure

```
OS90/
    APPS/
        (App name)/
            (name).ICO
            (name).PIF
    SYSTEM/
        INIT/
            STARTUP.CMD
        DRIVERS/
    BIN/
        CMD.EXE
    LIB/
    USER/
        DESKTOP/
        RECYCLE/
        DOCS/
```

# INIT System

OS/90 has a basic INIT system. Services are stored in SYSTEM/INIT and have a RUN.CMD script.

# Applications Folder

Installable programs are placed in the APPS directory. This is somewhat like macOS, which has an Applications folder. Programs can be deleted by simply using DELTREE and removing the folder. Installation is as simple as copying to the APPS directory.

An application can contain anything in this folder, as long as the executable and PIF file are present and have the same name as the program (with different extensions).

The shell is able to run programs in the APPS folder.

## PIF

PIF files work differently than they do in Windows 3.x. Direct hardware access is never allowed for any programs, unlike in Windows (old versions). Memory requirements are deduced by the kernel based on the executable header.

The format is in binary and uses 0xFF terminated strings or integers in this order:
* Name of the program
* version major and minor (one byte each, MJ, MN)
* Path to the icon (absolute or relative)
* Open fullscreen

Exec specifies the file that should be executed when the program runs. This must be in the root of the application.

# GUI

The GUI is similar to DESQview. It runs in a 640x480 display.
