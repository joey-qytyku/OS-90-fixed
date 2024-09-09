## Memory Chains


## History

August 4 2024: Removed a few functions. Now there are only four. Uncommitted size will be the business of whatever is using the chain. Page bits and other mapping information are removed from the kernel and the API. PAGE_PROC will not be a chain.

September 3:
Now page-based rather than byte-based. Why perform a pointless calculation if we already know how many pages?
