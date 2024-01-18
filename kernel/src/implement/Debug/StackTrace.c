#include <Debug/Debug.h>

// Use a linked list for stack trace data
// That way drivers can use it too

// Returns null if it is not recorded
static const char* LookupFunction(PVOID addr_of_function)
{}

VOID OutputStackTrace(VOID)
{
    KLogf("%s %s", __FUNCTION__, " caused a fatal error.");

    // To perform a stack trace, we simply follow the return address
    PU32 addr = __builtin_return_address(0);

    // Follow the first one and replace with followed one
    addr = (PU32)*addr;

    while (1) {
        const char * name = LookupFunction(addr);

        // If we cannot find the function in the lookup table, break out
        if (name == NULL)
            break;

        KLogf("  called by %s", name);

        // Go to next
        addr = (PU32)*addr;
    }
}

VOID EnableStackTraces()
{
}
