#ifndef CHAIN_H
#define CHAIN_H
#include "page.h"

typedef struct {
        WORD    rel_index   :15;
        WORD    f_inuse     :1;

        SIGSHORT    next;
        SIGSHORT    prev;
}MB,*P_MB;

LONG M_Alloc(LONG bytes_commit, LONG bytes_uncommit);
VOID M_Free(LONG chain);

SIGLONG M_ResizeWithCommit(LONG chain, SIGLONG delta_bytes);
SIGLONG M_ExtendUncommit(LONG bytes);

#endif /* CHAIN_H */
