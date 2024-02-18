#ifndef SCHEDULER_SYNC_H
// #define SCHEDULER_SYNC_H

#define _STI() { __asm__ volatile ("sti":::"memory"); }
#define _CLI() { __asm__ volatile ("cli":::"memory"); }

ALIGN(4)
typedef struct { int :32; } atomic_t;

#define ATOMIC_INIT {0}

// The m constraint takes the address of the symbol passed to it.
// If we pass the name of the pointer rather than the theoretical
// thing that it points to, it will not interpret it as the address of
// the target object.
//
// This causes GCC to think that we are trying to access the pointer argument
// itself and uses the stack pointer as an address.

__attribute__((always_inline))
static inline void release_mutex(atomic_t *m)
{
  __asm__ volatile (
    "btrl $0,%0":"=m"(*m)::"memory"
  );
}

__attribute__((always_inline))
static inline void acquire_mutex(atomic_t *m)
{
  __asm__ volatile (
    "spin%=:\n\t"
    "btsl $0,%0\n\t"
    "jc spin%="
    :"=m"(*m)
    :"m"(*m)
    :"memory"
  );
}

__attribute__((always_inline))
static inline _Bool mutex_was_locked(atomic_t *m)
{
  _Bool ret;
  __asm__ volatile(
    "cmpl $0, %1"
    :"=@ccz"(ret)
    :"m"(*m)
    :"memory","cc"
  );
  return ret;
}

__attribute__((always_inline))
static inline uint save_flags(void)
{
  uint ret;
  __asm__ volatile (
    "pushf\n\t"
    "pop %0"
    :"=rm"(ret)
    :
    :"memory", "cc"
  );
}

__attribute__((always_inline))
static inline void set_flags(uint flags)
{
  __asm__ volatile (
    "pushl %0"
    "popf"
    :
    :"rm"(flags)
    :"memory", "cc"
  );
}

__attribute__((always_inline))
static inline void atomic_fenced_store(atomic_t *address, uint value)
{
  __asm__ volatile(
    "movl %1,%0"
    :"=m"(*address)
    :"n"(value)
    :"memory"
  );
}

__attribute__((always_inline))
static inline uint atomic_fenced_load(atomic_t *address)
{
  uint ret;
  __asm__ volatile(
    "movl %1,%0"
    :"=r"(ret)
    :"m"(*address)
    :"memory"
  );
  return ret;
}

__attribute__((always_inline))
static inline void atomic_fenced_dec(atomic_t *address)
{
  __asm__ volatile ( "decl %0" :"=m"(*address) :"m"(*address) :"memory");
}

__attribute__((always_inline))
static inline void atomic_fenced_inc(atomic_t *address)
{
  __asm__ volatile ( "incl %0" :"=m"(*address) :"m"(*address) :"memory");
}

__attribute__((always_inline))
static inline _Bool atomic_fenced_compare(atomic_t *address, uint imm)
{
  _Bool ret;
  __asm__ volatile (
    "cmpl %1, %2"
    : "=@ccz"(ret)
    : "i"(imm), "m"(*address)
    : "cc", "memory"
  );
  return ret;
}

#define K_PreemptDec() atomic_fenced_dec(&g_preempt_count)
#define K_PreemptInc() atomic_fenced_inc(&g_preempt_count)

API_DECL(void, preempt_dec, void);
API_DECL(void, preempt_inc, void);

extern atomic_t g_preempt_count;

#endif /* SCHEDULER_SYNC_H */