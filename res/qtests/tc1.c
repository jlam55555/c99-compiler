int a;

int f(int b)
{
	b = a+3;
}

int g()
{
	struct x {
		int a1;
		int b2;
	} x;

	// struct members not supported
//	x.b2 = a;
}