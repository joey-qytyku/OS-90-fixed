/*******************************************************************************
                      Copyright (C) 2022-2024, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/

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

