# Mouse and Keyboard Driver (HID8042.DRV)

HID8042 arbitrates the keyboard and mouse. It emulates the entire hardware and software interfaces. It allows multiple programs to access the mouse concurrently.

# Mouse API

There exists a mouse API for DOS using INT 33h. The goal of this driver is to support DOS boxes where the user can click directly on a program that supports mouse features. The position of the mouse must be simulated by the userspace.

The mouse API returns the location of the mouse in X and Y. The X is a number from 0 to 639 and the Y is a number from 0 to 199. Video mode does not affect the values.

The API allows a program to set its own callback for IRQs. This requires that the address of this handler is maintained by the driver. Whether or not the IRQ is being directly handled with a fake IRQ or processed by the driver must also be recorded. When using the callback, the procedure is far called using V86.

# Event Calls

The major function code is one.
