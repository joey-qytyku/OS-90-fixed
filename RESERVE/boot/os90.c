long long int test(long long int a, long long int b)
{
	return a+b;
}

#pragma aux test __parm [__esi] [__edi]

int main()
{}
