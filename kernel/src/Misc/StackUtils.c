#include <Misc/StackUtils.h>
#include <Misc/Segutils.h>

VOID RmPush16(PWORD ss, PDWORD esp, WORD value)
{
    PWORD stack = MK_LP(*ss, *esp);
    *esp -= 2;
    stack[-1] = value;
}

WORD RmPop16(PWORD ss, PDWORD esp)
{
    PWORD stack = MK_LP(*ss, *esp);
    *esp += 2;
    return *stack;
}

VOID PmPush16(PWORD ss, PDWORD esp, WORD value)
{
    PWORD stack = GetLdescBaseAddress(ss) + *esp;
    *esp -= 2;
    stack[-1] = value;
}

WORD PmPop16(PWORD ss, PDWORD esp)
{
    PWORD stack = GetLdescBaseAddress(ss) + *esp;
    *esp += 2;
    return *stack;
}
