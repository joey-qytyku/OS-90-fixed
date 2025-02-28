This folder contains implementation files and headers for C-library functions that can run in kernel mode and user mode using certain defines.

Some are more portable than others. printf can be used on arm/x64/i386 while malloc is architecturally optimized.

The implementation is simply included into a C file.

```
// printf.c in any module that needs it does this

#define DISABLE_FLOAT
#include "../SHARED/printf/printf.c"
```

There are also no include guards in the headers. They must be inserted separately.

The headers are also ONLY for exporting, not for importing.

## printf

This module is highly portable. It can run under many operating systems and only depends on the C library if required. Currently 64-bit and 32-bit targets are supported.

- DISABLE_FLOAT: If defined, floating point is turned off and float conversions are an error
- PUTCHAR_COMMIT: name of the function to call for printing characters. Comes with a default one if not defined that using fwrite and fputc.

_printf_core is non-compliant and is used to implement the entire family of formatting calls. It is like vsnprintf but with a buffer commit callback.
