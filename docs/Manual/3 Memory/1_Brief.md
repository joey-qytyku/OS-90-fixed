# Memory Manager

OS/90 handles memory using a page size of 4K. There are no plans to support transparent hugepages like on Linux, but such a feature is possible since all operations use bytes and are rounded to pages.
