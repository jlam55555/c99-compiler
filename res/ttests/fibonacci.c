// calculate nth term of fibonacci sequence
long fib(int n)
{
	long cur, next, tmp;
	int i;

	if (n == 0) {
		return (long)0;
	}

	// this is to ensure top bits get properly zero-filled
	cur = 0;
	next = 1;
	i = 1;

	while (i < n) {
		tmp = cur;
		cur = next;
		next = cur + tmp;
		++i;
	}

	return next;
}