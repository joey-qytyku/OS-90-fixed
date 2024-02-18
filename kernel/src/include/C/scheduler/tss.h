#ifndef TSS_H
#define TSS_H

enum {
  TSS_ESP0 = 1,
  TSS_SS0  = 2,
  TSS_SV86_EBP = 3,
  TSS_SV86_EIP = 4
};

extern uint tss[26] asm("adwTaskStateSegment");

#endif /*TSS_H*/
