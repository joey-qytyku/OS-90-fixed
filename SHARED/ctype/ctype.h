extern unsigned char __ctype_lut[257];

// If optimizations are off, do not force inline.
// These can usually be

#ifdef __OPTIMIZE__
	#define DECL static inline __attribute__((force_inline)) int
	#define DEF(x) x
#else
	#define DECL
	#define DEF(x) ;
#endif

DECL isalpha(int c)	DEF({ return !!((__ctype_lut[c+1]) & 1  ) ;})
DECL islower(int c)	DEF({ return !!((__ctype_lut[c+1]) & 2  ) ;})
DECL isupper(int c)	DEF({ return!!!((__ctype_lut[c+1]) & 2  ) ;})
DECL isdigit(int c)	DEF({ return !!((__ctype_lut[c+1]) & 4  ) ;})
DECL isxdigit(int c)	DEF({ return !!((__ctype_lut[c+1]) & 8  ) ;})
DECL iscntrl(int c)	DEF({ return !!((__ctype_lut[c+1]) & 16 ) ;})
DECL isgraph(int c)	DEF({ return !!((__ctype_lut[c+1]) & 32 ) ;})
DECL isspace(int c)	DEF({ return !!((__ctype_lut[c+1]) & 64 ) ;})
DECL isblank(int c)	DEF({ return !!((__ctype_lut[c+1]) & 128) ;})
DECL isprint(int c)	DEF({ return !!((__ctype_lut[c+1]) & 256) ;})
DECL ispunct(int c)	DEF({ return !!((__ctype_lut[c+1]) & 512) ;})

#undef DECL
