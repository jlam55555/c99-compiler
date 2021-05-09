int f(int *a)
{
	*a += 2;
}

int main()
{
	int a;
    a=0;
	printf("%d\n", f(&a));

	printf("%d\n", a);

	printf("%d\n", g());
}