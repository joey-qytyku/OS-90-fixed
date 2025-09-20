# M_LockPageTables

```c
uint* M_LockPageTables(void);
```

Asserts the lock responsible for the page tables (it may be the same as MM).

The page tables are laid out flat in memory. It is returned by this pointer.

Never pass the pointer to another thread. The pointer becomes invalid upon unlocking.
