# Page Attribute Table

The PAT has many useful applications:
- Faster framebuffer access with write combining
- Reduced cache polution for memcpy-style operations

Enabling PAT turns bits `PCD PWT` into an index to a table of other options, which are by default set for compatibility with OS'es that use the PAT bit but have no knowledge of it. There are four possible options.

The PAT options are:
- 0: UC- or uncached [1]
- 1: WC: Write-combine [2]
- 2: WP: Write-protect [3]
- 3: UC+

[1] The reason for this is that the MTTRs are often set by the BIOS and we do not want to interfere with it. Use this most of the time

[2] WC is best for memory regions that are frequently written into non-sequentially and over existing data. It stores the writes in a lower cache level and writes back the results in a burst access. Framebuffers are a notable example, although

[3] Not sure how different this is from writethrough

Uncached regions are very useful for performance as not all computers have non-temporal operations. memcpy-style operations could in theory be faster this way because cache is not wasted.
