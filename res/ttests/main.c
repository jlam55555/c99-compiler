// for esieve
static int buf[10000];
extern int multidim[5][5][5];

int main(void)
{
	// Hello, world!
	printf("Hello, world!\n");

	// calling an external fn that returns something
	printf("%d\n", f());

	// calling an external function which takes a parameter
	int b;
	b = 52;
	add_two(&b);
	printf("b+2: %d\n", b);

	// multidim arrays!
	int i, j, k;
	multidim_test();
	for (i=0; i<5; ++i) {
		printf("slice %d:\n", i);

		for (j=0; j<5; ++j) {
			for (k=0; k<5; ++k) {
				printf("%2d ", multidim[i][j][k]);
			}
			printf("\n");
		}
		printf("\n");
	}

	// sieve of eratosthenes!
	int prime_count, max;
	max = 1000;

	esieve(max, buf, &prime_count);
	printf("Got %d primes up to %d:\n", prime_count, max);
	for (i=0; i<prime_count; ++i) {
		printf("%4d ", buf[i]);
	}
	printf("\n");

	// implicit ret type of function is int, but we want long here, so
	// have to predeclare
	long fib(int);
	printf("%dth fibonacci number: %ld\n", 75, fib(75));

	return 0;
}
