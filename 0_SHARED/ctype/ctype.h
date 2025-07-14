#ifndef SHARED_CTYPE
#define SHARED_CTYPE

extern unsigned short __ctype_lut[257];

// If optimizations are off, do not force inline.
// These can usually be

#define isalpha(c)	( !!((__ctype_lut[(int)c+1]) & 1  ) )
#define islower(c)	( !!((__ctype_lut[(int)c+1]) & 2  ) )
#define isupper(c)	(!!!((__ctype_lut[(int)c+1]) & 2  ) )
#define isdigit(c)	( !!((__ctype_lut[(int)c+1]) & 4  ) )
#define isxdigit(c)	( !!((__ctype_lut[(int)c+1]) & 8  ) )
#define iscntrl(c)	( !!((__ctype_lut[(int)c+1]) & 16 ) )
#define isgraph(c)	( !!((__ctype_lut[(int)c+1]) & 32 ) )
#define isspace(c)	( !!((__ctype_lut[(int)c+1]) & 64 ) )
#define isblank(c)	( !!((__ctype_lut[(int)c+1]) & 128) )
#define isprint(c)	( !!((__ctype_lut[(int)c+1]) & 256) )
#define ispunct(c)	( !!((__ctype_lut[(int)c+1]) & 512) )

#endif /*SHARED_CTYPE*/
