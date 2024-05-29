/*
  ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
  บ                  Copyright (C) 2023-2028, Joey Qytyku                    บ
  ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
  บ This file is part of OS/90 and is published under the GNU General Public บ
  บ   License version 2. A copy of this license should be included with the  บ
  บ     source code and can be found at <https://www.gnu.org/licenses/>.     บ
  ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

#ifndef BASICATOMIC_H
#define BASICATOMIC_H

#define STI()                                         \
        {                                             \
                __asm__ volatile("sti" ::: "memory"); \
        }
#define CLI()                                         \
        {                                             \
                __asm__ volatile("cli" ::: "memory"); \
        }

typedef ALIGN(4) LONG ATOMIC32;

#define ATOMIC32_INIT { 0 }

// The m constraint takes the address of the symbol passed to it.
// If we pass the name of the pointer rather than the theoretical
// thing that it points to, it will not interpret it as the address of
// the target object.
//
// This causes GCC to think that we are trying to access the pointer argument
// itself and uses the stack pointer as an address.

__attribute__((always_inline))
static inline VOID RELEASE_MUTEX(ATOMIC32 *m)
{
        __asm__ volatile(
            "btrl $0,%0" : "=m"(*m)::"memory");
}

__attribute__((always_inline))
static inline VOID ACQUIRE_MUTEX(ATOMIC32 *m)
{
        __asm__ volatile(
            "spin%=:\n\t"
            "btsl $0,%0\n\t"
            "jc spin%="
            : "=m"(*m)
            : "m"(*m)
            : "memory");
}

__attribute__((always_inline))
static inline BOOL MUTEX_WAS_LOCKED(ATOMIC32 *m)
{
        BOOL ret;
        __asm__ volatile(
            "cmpl $0, %1"
            : "=@ccz"(ret)
            : "m"(*m)
            : "memory", "cc");
        return ret;
}

__attribute__((always_inline))
static inline LONG SAVE_FLAGS(VOID)
{
        LONG ret;
        __asm__ volatile(
            "pushf\n\t"
            "pop %0"
            : "=rm"(ret)
            :
            : "memory", "cc");
        return ret;
}

__attribute__((always_inline))
static inline VOID SET_FLAGS(LONG flags)
{
        __asm__ volatile(
            "pushl %0\n"
            "popf"
            :
            : "rm"(flags)
            : "memory", "cc");
}

__attribute__((always_inline))
static inline VOID ATOMIC32_FENCED_STORE(ATOMIC32 *address, LONG value)
{
        __asm__ volatile(
            "movl %1,%0"
            : "=m"(*address)
            : "n"(value)
            : "memory");
}

__attribute__((always_inline))
static inline LONG ATOMIC32_FENCED_LOAD(ATOMIC32 *address)
{
        LONG ret;
        __asm__ volatile(
            "movl %1,%0"
            : "=r"(ret)
            : "m"(*address)
            : "memory");
        return ret;
}

__attribute__((always_inline))
static inline VOID ATOMIC32_FENCED_DEC(ATOMIC32 *address)
{
        __asm__ volatile("decl %0" : "=m"(*address) : "m"(*address) : "memory");
}

__attribute__((always_inline))
static inline VOID ATOMIC32_FENCED_INC(ATOMIC32 *address)
{
        __asm__ volatile("incl %0" : "=m"(*address) : "m"(*address) : "memory");
}

__attribute__((always_inline))
static inline BOOL ATOMIC32_FENCED_COMPARE32(ATOMIC32 *address, LONG imm)
{
        BOOL ret;
        __asm__ volatile(
            "cmpl %1, %2"
            : "=@ccz"(ret)
            : "i"(imm), "m"(*address)
            : "cc", "memory");
        return ret;
}

#define K_PreemptDec() ATOMIC32_FENCED_DEC(&g_preempt_count)
#define K_PreemptInc() ATOMIC32_FENCED_INC(&g_preempt_count)

#define IRQ_OFF_SECTION(num) \
        FENCE();\
        LONG flags__##num = SAVE_FLAGS();\
        FENCE();\
        _CLI();

#define END_IRQ_OFF(num) \
        SET_FLAGS(flags__##num);\

extern LONG g_preempt_count;

#endif /* BASICATOMIC_H */
