# Memory Information

The memory manager has M_Query for getting all information related to the memory manager.

```
P_MM_STRUCT M_Query(VOID);
```

Never under ANY circumstance change a single entry in the structure. Do not touch anything kernel-related.

Keep in mind that the accuracy of this information is not perfect because it could change at any time. Consider disabling preemption if the information is critical.

The lock can be checked to see if it is currently locked.
