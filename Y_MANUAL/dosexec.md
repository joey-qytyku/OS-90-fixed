# DOS Virtual Machines

> Good ideas, but still not really operational

OS/90 creates a DOS virtual machine context which contains all states related to services controlled by the OS. It is impossible to use fixed parameters to define every option. Instead, a list of initialization options is passed to the kernel that is then parsed to set relevant options.

This document specifies each option.

# Notes

No options are required. Each has an automatic default.

Subprocesses are currently subject to exactly the same rules as the parent process even though they get entire contexts of their own.

OS/90 has no fixed restrictions on the number of threads that can be active in a DOS process (yes it can multithread), XMS handles (aside from 16-bit limits), file handles, and many other resources. The OS uses fixed arrays for data if they fit and more are allocated as needed.


	The Interface


CreateVM is the ultimate interface for creating virtual machines. It is available on userspace using a special DLL that remains undocumented but uses an INT vector.

```
void *CreateVM
(
	const char *exec_path,
	const char *command_tail,
	void *init_array
);
```

This returns a VM ID as a void pointer, which is opaque and should not be modified or casted and then dereferenced.

The result is OSNULL if it failed, which can be because the range of an argument was wrong or something was null that should not have been.

```
	EXTENDED_PERCENT_MAX(/*percentage*/))
```

The percentage of extended memory still available at the time is creating the process which can be allocated. The percentage is written as a simple integer.

This is approximate. The amount of extended memory allowed is not updated later.


If zero, the program has access to no extended memory

```
	CONVENTIONAL_KB(/*integer*/),
```

Number of KB that can be allocated for the program. This INCLUDES the program segment. The value should be thought of as exact.

The MZ header option for allocating extra memory to the program segment is also a factor. If this option is overrun, the program will fail to execute.

................................................................................
	PAGE_FILE_PERCENT_MAX(/*percentage*/),
................................................................................

The amount of demand-paged disk backed memory that is allowed to be allocated. This should be ZERO if the program is allowed 100% of the main memory as some programs allocate everything and immediately begin paging past that.

................................................................................
	INITIAL_VIDEO_MODE(/*valid VGA text mode*/),
................................................................................

Sets the initial VGA mode. This MUST be a text mode.

................................................................................
	FRAMEBUFFER_SHARE_PTR(/*pointer to pointer*/),
	FRAMEBUFFER_WRITE_COMBINE(/*boolean*/)
................................................................................

................................................................................
	DISALLOW_VECTOR(/*Int vector*/)
	DISALLOW_SERVICE(/*?????*/);
................................................................................

................................................................................
	PERMIT_DIRECT_IRQ(/*IRQ Mask*/)
................................................................................

................................................................................
	STDIO_HANDLES(in, out, err, prn)
	STDIO_ENABLE_COOKED_IFNOMODESET
................................................................................

This sets the alias targets of the standard IO handles. PRN is WIP.



IRQ#0 cannot be controlled directly by the VM at the moment, so the bit should be off. Parameter is a bit mask that decides which interrupt vectors can be directly accessed IF AND ONLY IF they are not claimed by a ring-0 driver.

If a driver is using it, the vector has to be virtualized by the driver or the program must be terminated, This is out of the scope of the document.

The pointer to share the framebuffer address with. Guaranteed to be page-aligned and writable in all modes.

> How do we handle a growing framebuffer? Also we may want to handle graphical modes too. Maybe I want to write a GUI instead or extend ATM to support graphics?

The framebuffer can be marked as write combining. Not recommended and will probably remove.

    SWAP_PERCENT_MAX(20),
    STDIO_HANDLES(in, out, err, prn),
    STDIO_ENABLE_COOKED_IFNOMODESET,
    USERSPACE_RM_HOOKS(...),
    USERSPACE_PM_HOOKS(...),
    FRAMEBUFFER_SHARE_PTR(&fbptr),
    END_OF_EXEC_REQUEST
> A default initialization array exists for simplicity. This is slow?

// NOTE: the sentinel is marked by an attribute?
