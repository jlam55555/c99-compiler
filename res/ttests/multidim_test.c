int multidim[5][5][5];

void multidim_test()
{
	int i, j, k;

	for (i=0; i<5; ++i) {
		for (j=0; j<5; ++j) {
			for (k=0; k<5; ++k) {
				multidim[i][j][k] = i+j+k;
			}
		}
	}
}