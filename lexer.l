%option noyywrap
%x STRMODE
%x CHRMODE
%{
#include "tokensmanual.h"

enum {
	CHSTART = 0x801,
	CHEND,
	STSTART,
	STEND,
	TEXT,
	SIMESC,
	OCTESC,
	HEXESC
};

// union yylval {
// 	int i;
// }
int yylval;
%}
%%
asdfklmafklm	{return IDENT; /* TODO */}

0[xX][0-9a-fA-F]+[pe]*[-]*[0-9]+		{return NUMBER;}
[-+]?[0-9]*[.]?[0-9]*[pe]?[-]?[0-9]+[LU]?	{return NUMBER;}


[uUL]?\'			{/* these differ between chars/strings */
				 BEGIN CHRMODE; return CHSTART;}
<CHRMODE>[^\'\\\n]+		{return TEXT;}
<CHRMODE>\'			{BEGIN INITIAL; return CHEND;}
[(u8)uUL]?\"			{BEGIN STRMODE; return STSTART;}
<STRMODE>[^\"\\\n]+		{return TEXT;}
<STRMODE>\"			{BEGIN INITIAL; return STEND;}

<CHRMODE>\\['"\?\\abfnrtv]	{/* these are the same for chars/strings */
				 return SIMESC;}
<CHRMODE>\\[0-8]{1,3}		{return OCTESC;}
<CHRMODE>\\x[0-9a-fA-F]+	{return HEXESC;}
<STRMODE>\\['"\?\\abfnrtv]	{return SIMESC;}
<STRMODE>\\[0-8]{1,3}		{return OCTESC;}
<STRMODE>\\x[0-9a-fA-F]+	{return HEXESC;}

indsel		{return INDSEL;}
\+\+		{return PLUSPLUS;}
--		{return MINUSMINUS;}
\<\<		{return SHL;}
>>		{return SHR;}
\<=		{return LTEQ;}
>=		{return GTEQ;}
==		{return EQEQ;}
!=		{return NOTEQ;}
&&		{return LOGAND;}
\|\|		{return LOGOR;}
\.\.		{return ELLIPSIS;}
\*=		{return TIMESEQ;}
\/=		{return DIVEQ;}
%=		{return MODEQ;}
\+=		{return PLUSEQ;}
-=		{return MINUSEQ;}
\<\<=		{return SHLEQ;}
>>=		{return SHREQ;}
&=		{return ANDEQ;}
\|=		{return OREQ;}
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

[!#%\^&\*\(\)\-\=\+\[\]\{\}\|\;\<\>\.\?] {return yytext[0];}

[ \t\n\v\f\r]+	{; /* ignore whitespace */}

.		{ /* TODO: handle error */ ;}
%%
// TODO: escaped characters?
// TODO: digraphs?

int main() {
	int t;

	while (t = yylex()) {
		fprintf(stdout, "%s: %d\n", yytext, t);
		switch (t) {
			case NUMBER:
				fprintf(stdout, "Number found\n");
				break;
		}
	}

	return 0;
}