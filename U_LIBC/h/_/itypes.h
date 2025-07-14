#ifndef __ITYPES_H
#define __ITYPES_H

typedef __SIZE_TYPE__		size_t;
typedef __PTRDIFF_TYPE__	ptrdiff_t;
typedef __WCHAR_TYPE__		wchar_t;
typedef __WINT_TYPE__		wint_t;
typedef __INTMAX_TYPE__		intmax_t;
typedef __UINTMAX_TYPE__	uintmax_t;
typedef __SIG_ATOMIC_TYPE__	sig_atomic_t;
typedef __INT8_TYPE__		int8_t;
typedef __INT16_TYPE__		int16_t;
typedef __INT32_TYPE__		int32_t;
typedef __INT64_TYPE__		int64_t;
typedef __UINT8_TYPE__		uint8_t;
typedef __UINT16_TYPE__		uint16_t;
typedef __UINT32_TYPE__		uint32_t;
typedef __UINT64_TYPE__		uint64_t;
typedef __INT_LEAST8_TYPE__	int_least8_t;
typedef __INT_LEAST16_TYPE__	int_least16_t;
typedef __INT_LEAST32_TYPE__	int_least32_t;
typedef __INT_LEAST64_TYPE__	int_least64_t;
typedef __UINT_LEAST8_TYPE__	uint_least8_t;
typedef __UINT_LEAST16_TYPE__	uint_least16_t;
typedef __UINT_LEAST32_TYPE__	uint_least32_t;
typedef __UINT_LEAST64_TYPE__	uint_least64_t;
typedef __INT_FAST8_TYPE__	int_fast8_t;
typedef __INT_FAST16_TYPE__	int_fast16_t;
typedef __INT_FAST32_TYPE__	int_fast32_t;
typedef __INT_FAST64_TYPE__	int_fast64_t;
typedef __UINT_FAST8_TYPE__	uint_fast8_t;
typedef __UINT_FAST16_TYPE__	uint_fast16_t;
typedef __UINT_FAST32_TYPE__	uint_fast32_t;
typedef __UINT_FAST64_TYPE__	uint_fast64_t;
typedef __INTPTR_TYPE__		intptr_t;
typedef __UINTPTR_TYPE__	uintptr_t;

#define CHAR_BIT __CHAR_BIT__

#define INT8_C		__INT8_C
#define INT16_C		__INT16_C
#define INT32_C		__INT32_C
#define INT64_C		__INT64_C
#define UINT8_C		__UINT8_C
#define UINT16_C	__UINT16_C
#define UINT32_C	__UINT32_C
#define UINT64_C	__UINT64_C
#define INTMAX_C	__INTMAX_C
#define UINTMAX_C	__UINTMAX_C

#endif /*__ITYPES_H*/
