#define printf(fmt, ...) { \
	if (sizeof(_STR(__VA_ARGS__)<=1))\
		{puts(fmt);} } \
	else	{_printf(fmt, __VA_ARGS__);}

