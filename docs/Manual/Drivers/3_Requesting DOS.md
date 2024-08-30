## Requesting DOS Services

INTxH is the general interface for calling BIOS and DOS functions. There are many reasons to use such functions, such as for reading the disk without knowing the disk driver or accessing files such as logs or configurations.

There are a few notes on how drivers should use it.

DOS uses the Program Segment Prefix to make certain program-local operations work properly. For example, files. There are also contexts that are global but must be made local, such as the current directory, which is stored individually for each disk by DOS and OS/90.

This means that certain operations require the use of a temporary emulation context that does not disrupt the rest of the system.

> Is this all necessary? The drivers should never try to change the current directory anyway.
