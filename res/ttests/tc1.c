// for esieve
static int buf[10000];

int main(void)
{
	// Hello, world!
	printf("Hello, world!\n");

	// calling an external fn
	printf("%d\n", f());

	// siece of eratosthenes!
	int prime_count, max, i;
	max = 1000;

	esieve(max, buf, &prime_count);
	printf("Got %d primes up to %d:\n", prime_count, max);
	for (i=0; i<prime_count; ++i) {
		printf("%4d ", buf[i]);
	}
	printf("\n");

	// currently have a problem with returning long values to intermediates;
	// have to use the long result of a fncall in a direct assignment
	long fib75;
	fib75 = fib(75);
	printf("%dth fibonacci number: %ld\n", 75, fib75);

	return 0;
}
