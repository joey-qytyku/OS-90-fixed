
//
// Enhanced mutex will yield to a process with the PTASK stored inside it.
// This allows for a rapid response to a change in the lock state.
//
// An 8-byte structure is used.
//

typedef struct {
        LONG a, b;
}ENHMTX,*PENHMTX;

VOID AcquireEnhMutex(PENHMTX m);

VOID ReleaseEnhMutex(PENHMTX m);

