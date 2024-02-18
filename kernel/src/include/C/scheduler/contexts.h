#ifndef CONTEXTS_H
#define CONTEXTS_H

#define _STI { __asm__ volatile ("sti":::"memory"); }
#define _CLI { __asm__ volatile ("cli":::"memory"); }


// Maybe rename mov to _mov_r_rm
#define _push(var) __asm__ volatile ("pushl %0"::"rm"(var):"memory")
#define _mov(d,s)  __asm__ volatile ("movl %0,%1":"=r"(d):"rm"(s):"memory")

#define LOAD_GENERAL_REGS(regs)\
_mov(_EAX, regs->EAX);\
_mov(_EBX, regs->EBX);\
_mov(_ECX, regs->ECX);\
_mov(_EDX, regs->EDX);\
_mov(_ESI, regs->ESI);\
_mov(_EDI, regs->EDI);\
_mov(_EBP, regs->EBP)

#define IRET() __asm__ volatile ("iret":::"memory");

#define BUILD_RINGSWITCH_STKFRAME(regs)\
_push(regs->SS);\
_push(regs->ESP);\
_push(regs->EFLAGS);\
_push(regs->CS);\
_push(regs->EIP)

#define BUILD_V86_DSEGS(regs)\
_push(regs->v86_ES);\
_push(regs->v86_DS);\
_push(regs->v86_FS);\
_push(regs->v86_GS)

register uint _EAX asm("eax");
register uint _EBX asm("ebx");
register uint _ECX asm("ecx");
register uint _EDX asm("edx");
register uint _ESI asm("esi");
register uint _EDI asm("edi");
register uint _EBP asm("ebp");
register uint _ESP asm("esp");
#endif /*CONTEXTS_H*/