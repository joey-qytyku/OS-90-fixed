## Address Spaces Explained

There is one shared address space. The real mode addressable region including 65536 bytes above 1M are unpaged, unswappable, and permanently mapped. As a result, real mode memory is shared by all DOS programs. This reduces available memory but improves performance because there is no need to translate from real mode to real mode.
