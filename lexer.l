%option noyywrap
%{
#include "tokensmanual.h"

// union yylval {
// 	int i;
// }
int yylval;
%}
%%
[0-9]+		{return NUMBER;}
CHARLIT		{;}
STRING		{;}

indsel		{return INDSEL;}
\+\+		{return PLUSPLUS;}
--			{return MINUSMINUS;}
\<\<			{return SHL;}
>>			{return SHR;}
\<=			{return LTEQ;}
>=			{return GTEQ;}
==			{return EQEQ;}
!=			{return NOTEQ;}
&&			{return LOGAND;}
\|\|			{return LOGOR;}
\.\.\.			{return ELLIPSIS;}
\*=			{return TIMESEQ;}
\/=			{return DIVEQ;}
%=			{return MODEQ;}
\+=			{return PLUSEQ;}
-=			{return MINUSEQ;}
\<\<=			{return SHLEQ;}
>>=			{return SHREQ;}
&=			{return ANDEQ;}
\|=			{return OREQ;}
xoreq		{return XOREQ;}
auto		{return AUTO;}
break		{return BREAK;}
case		{return CASE;}
char		{return CHAR;}
continue	{return CONTINUE;}
default		{return DEFAULT;}
do		{return DO;}
double		{return DOUBLE;}
else		{return ELSE;}
enum		{return ENUM;}
extern		{return EXTERN;}
float		{return FLOAT;}
for		{return FOR;}
goto		{return GOTO;}
if		{return IF;}
inline		{return INLINE;}
int		{return INT;}
long		{return LONG;}
register	{return REGISTER;}
restrict	{return RESTRICT;}
return		{return RETURN;}
short		{return SHORT;}
signed		{return SIGNED;}
sizeof		{return SIZEOF;}
static		{return STATIC;}
struct		{return STRUCT;}
switch		{return SWITCH;}
typedef		{return TYPEDEF;}
union		{return UNION;}
unsigned	{return UNSIGNED;}
void		{return VOID;}
volatile	{return VOLATILE;}
while		{return WHILE;}
_bool		{return _BOOL;}
_complex	{return _COMPLEX;}
_imaginary	{return _IMAGINARY;}

!		{return '!';}
#		{return '#';}
%		{return '%';}
\^		{return '^';}
&		{return '&';}
\*		{return '*';}
\(		{return '(';}
\)		{return ')';}
-		{return '-';}
\=		{return '=';}
\+		{return '+';}
\[		{return '[';}
\]		{return ']';}
\{		{return '{';}
\}		{return '}';}
\|		{return '|';}
\;		{return ';';}
\<		{return '<';}
\>		{return '>';}
\.		{return '.';}
\?		{return '?';}

[ \t\n\v\f\r]+	{;}

.		{ /* TODO: handle error */ ;}
%%
// TODO: escaped characters?

int main() {
	int t;

	while (t = yylex()) {
		fprintf(stdout, "%s: ", yytext);
		switch (t) {
			case NUMBER:
				fprintf(stdout, "Number found\n");
				break;
			default:
				fprintf(stderr, "Symbol not found\n");
		}
	}

	return 0;
}