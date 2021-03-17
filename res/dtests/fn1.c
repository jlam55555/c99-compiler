int f();
int h(int);

int f(i)	/* non-prototype definition should be compat with prev decl */
double i;	/* compiler shouldn't notice this error */
{
 int j;
}

void (*g(double d))(int,char)
{
 int f();
 int g();	/* gcc complains about this but I think this g should
			hide the name g in global scope */
	_whatis g;		/* _whatis is my debug extension */

}

int h(int x)
{
 int h;		/* gcc does NOT complain about this */
}

kr(i,j,k)
double k;
{
	_whatis i;
	_whatis k;
}
