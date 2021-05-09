static int composites[10000];

// sieve of eratosthenes; n should be <= 10000
int esieve(int n, int *buf, int *count)
{
	int i, j;

	*count = 0;

	// clear composites
	for (i = 0; i < 10000; ++i) {
		composites[i] = 0;
	}

	// perform sieve up to sqrt(n)
	for (i = 2; i*i <= n; ++i) {
		if (composites[i] == 0) {
			buf[++*count-1] = i;
			for (j = i*i; j < n; j += i) {
				composites[j] = 1;
			}
		}
	}

	// get sieved primes
	for (; i < n; ++i) {
		if (composites[i] == 0) {
			buf[++*count-1] = i;
		}
	}
}