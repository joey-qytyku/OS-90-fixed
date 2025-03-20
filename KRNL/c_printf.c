unsigned int atoi(const char *s)
{
	static const unsigned int lookup[] =
	{1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};

	unsigned int final = 0;

	if (s == NULL)
		return 0;

	size_t len = strlen(s);

        unsigned int n = 0;

	for (size_t i = 0; i < len; i++) {
		if (!isdigit(s[i])) {
			goto leave;
		}
		else if (s[i] == '-') {
			n=1;
			continue;
		}
		else if (s[i] == ' ')
			continue;
		final += lookup[len-i-1] * (unsigned)(s[i]-'0');
	}
	leave:
	return n ? -final : final;
}

#include "../SHARED/printf/"
