// calculate nth term of fibonacci sequence
long fib(int n)
{
	long cur, next, tmp;
	int i;

	if (n == 0) {
		return 0;
	}

	// this is to ensure top bits get properly zero-filled
	cur = (long)0;
	next = (long)1;
	i = (long)1;

	while (i < n) {
		tmp = cur;
		cur = next;
		next = cur + tmp;
		++i;
	}

	return next;
}