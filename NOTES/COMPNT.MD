# Intermodular Component Interconnect (ICI)

> Work in progress.

## Goals

- Establish a direct mapping between a system-wide settings configuration app and drivers.
- Allow kernel and user mode to access settings
- Event driven methods of getting and setting options
- Options for built-in documentation for all settings
- Automatic driver-dependent settings if available
- Handling of dependent or incompatible settings
- Binding with drivers and user programs
- All settings are related to a GUI
- Insertion and removal of components
- Options saved to disk if requested.

ICI is not portable to other systems and is designed only for OS/90.

ICI is entirely implemented in the kernel.

## Example Default Hierarchy

```
System
	Kernel
		[Removable?]:   No
		[Files]:        ""

	User
		Libraries
			LIBC90.DLL
```

