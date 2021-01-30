%option noyywrap
%x STRMODE
%x CHRMODE
%{
#include "tokensmanual.h"
#include "stringutils.h"

// TODO: choose appropriate types
typedef union {
	int i;
	char c;
	float f;
	double d;
	char *s;
} YYSTYPE;

YYSTYPE yylval;
%}

DEC		[1-9]+[0-9]*
EXP 		[eE][+-]?{DEC}+
OCT 		0[0-7]+
HEX 		0[xX][0-9A-Fa-f]+
P 		[pP][+-]?{DEC}+
REAL 		[0-9]*\.[0-9]*

SIMESC		\\['"\?\\abfnrtv]
OCTESC		\\[0-7]{1,3}	
HEXESC		\\x[0-9a-fA-F]+

%%
 /* NUMBER */
{DEC}		{return NUMBER;}
{DEC}U		{return NUMBER;}
{DEC}L		{return NUMBER;}
{DEC}LU		{return NUMBER;}
{DEC}LL		{return NUMBER;}
{DEC}LLU		{return NUMBER;}
[0]?{DEC}+{EXP}[f]?	{return NUMBER;}

{OCT}		{return NUMBER;}
{OCT}U		{return NUMBER;}
{OCT}L		{return NUMBER;}
{OCT}LU		{return NUMBER;}
{OCT}LL		{return NUMBER;}
{OCT}LLU		{return NUMBER;}

{HEX}		{return NUMBER;}
{HEX}U		{return NUMBER;}
{HEX}L		{return NUMBER;}
{HEX}LU		{return NUMBER;}
{HEX}LL		{return NUMBER;}
{HEX}LLU		{return NUMBER;}
{HEX}{P}	{return NUMBER;}

{REAL}		{return NUMBER;}
{REAL}L 	{return NUMBER;}
{REAL}f		{return NUMBER;}

 /* STRING and CHARLIT; these differ between the two */
[uUL]?\'		{begin_string(yytext, 0); BEGIN CHRMODE;}
<CHRMODE>[^\'\\\n]+	{append_text(yytext);}
<CHRMODE>\'		{yylval.s = end_string();
			 BEGIN INITIAL; return STRING;}
[(u8)uUL]?\"		{begin_string(yytext, 1); BEGIN STRMODE;}
<STRMODE>[^\"\\\n]+	{append_text(yytext);}
<STRMODE>\"		{yylval.s = end_string();
			 BEGIN INITIAL; return STRING;}

 /* STRING and CHARLIT: these escape sequences are common between the two */
<CHRMODE>{SIMESC}	{/* these are the same for chars/strings */
			 parse_append_escape(yytext);}
<CHRMODE>{OCTESC}	{parse_append_octal(yytext);}
<CHRMODE>{HEXESC}	{parse_append_hexadecimal(yytext);}
<STRMODE>{SIMESC}	{parse_append_escape(yytext);}
<STRMODE>{OCTESC}	{parse_append_octal(yytext);}
<STRMODE>{HEXESC}	{parse_append_hexadecimal(yytext);}

 /* STRING and CHARLIT errors */
<STRMODE>.		{/* TODO: fix error handling */
			 fprintf(stderr, "Invalid string character found\n");}
<CHRMODE>.		{fprintf(stderr, "Invalid character literal found\n");}

 /* */
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

[a-zA-Z_][a-zA-Z0-9_]*	{yylval.s = yytext; return IDENT;}

[!#%\^&\*\(\)\-\=\+\[\]\{\}\|\;\<\>\.\?] {return yytext[0];}

[ \t\n\v\f\r]+	{; /* ignore whitespace */}

.		{fprintf(stderr, "Lexical error: '%c' encountered\n", yytext[0]);
		 /* TODO: handle error better */}
%%
// TODO: escaped characters?
// TODO: digraphs?
// TODO: is it okay to break up strings into many tokens like this?

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