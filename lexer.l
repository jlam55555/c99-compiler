%option noyywrap
%x STRMODE
%x CHRMODE
%{
#include <stdlib.h>
#include <stddef.h>
// #include "tokensmanual.h"
#include "stringutils.h"
#include "errorutils.h"
#include "numutils.h"
#include "parser.tab.h"

// typedef union {
// 	struct number num;
// 	char *ident;
// 	struct string string;
// 	struct charlit charlit;
// } YYSTYPE;

YYSTYPE yylval;

// helper for making integral and float types
#define MAKEINT(radix, sign, type) \
	yylval.num = make_int(radix, sign, type);\
	return NUMBER;
#define MAKEFP(type) \
	yylval.num = make_fp(type);\
	return NUMBER;
%}

DEC	[1-9]+[0-9]*
EXP 	[eE][+-]?{DEC}+
OCT 	0[0-7]*
HEX 	0[xX][0-9A-Fa-f]+
P	[pP][+-]?{DEC}+
REAL	([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+)

UL	([Ll][Uu]|[Uu][Ll])
LL	[Ll][Ll]	
ULL	([Uu](LL|ll)|(LL|ll)[Uu])	

SIMESC	\\['"\?\\abfnrtv]
OCTESC	\\[0-7]{1,3}	
HEXESC	\\x[0-9a-fA-F]+
%%
 /* Reading a preprocessor line number */
#[ ]{DEC}[ ]\"[^\"]+\".*\n	{parse_lineno();}

 /* Integer */
{DEC}		{MAKEINT(10, SIGNED_T, INT_T);}
{DEC}[Uu]	{MAKEINT(10, UNSIGNED_T, INT_T);}
{DEC}[Ll]	{MAKEINT(10, SIGNED_T, LONG_T);}
{DEC}{UL}	{MAKEINT(10, UNSIGNED_T, LONG_T);}
{DEC}{LL}	{MAKEINT(10, SIGNED_T, LONGLONG_T);}
{DEC}{ULL}	{MAKEINT(10, UNSIGNED_T, LONGLONG_T);}

{OCT}		{MAKEINT(8, SIGNED_T, INT_T);}
{OCT}[Uu]	{MAKEINT(8, UNSIGNED_T, INT_T);}
{OCT}[Ll]	{MAKEINT(8, SIGNED_T, LONG_T);}
{OCT}{UL}	{MAKEINT(8, UNSIGNED_T, LONG_T);}
{OCT}{LL}	{MAKEINT(8, SIGNED_T, LONGLONG_T);}
{OCT}{ULL}	{MAKEINT(8, UNSIGNED_T, LONGLONG_T);}

{HEX}		{MAKEINT(16, SIGNED_T, INT_T);}
{HEX}[Uu]	{MAKEINT(16, UNSIGNED_T, INT_T);}
{HEX}[Ll]	{MAKEINT(16, SIGNED_T, LONG_T);}
{HEX}{UL}	{MAKEINT(16, UNSIGNED_T, LONG_T);}
{HEX}{LL}	{MAKEINT(16, SIGNED_T, LONGLONG_T);}
{HEX}{ULL}	{MAKEINT(16, UNSIGNED_T, LONGLONG_T);}

 /* Float */
{REAL}		{MAKEFP(DOUBLE_T);}
{REAL}[Ll] 	{MAKEFP(LONGDOUBLE_T);}
{REAL}[Ff]	{MAKEFP(FLOAT_T);}
{HEX}{P}	{MAKEFP(DOUBLE_T);}
[0]?{DEC}+{EXP}[Ff]?	{char tmp=yytext[strlen(yytext)-1];
			 MAKEFP((tmp|32)=='f'?FLOAT_T:DOUBLE_T);}
[0]?{DEC}+{EXP}[Ll]?	{char tmp=yytext[strlen(yytext)-1];
			 MAKEFP((tmp|32)=='l'?LONGDOUBLE_T:DOUBLE_T);}

 /* STRING and CHARLIT; these differ between the two */
[uUL]?\'		{begin_literal(); BEGIN CHRMODE;}
<CHRMODE>[^\'\\\n]+	{append_text();}
<CHRMODE>\'		{yylval.charlit = end_charlit();
			 BEGIN INITIAL; return CHARLIT;}
(u8|[uUL])?\"		{begin_literal(); BEGIN STRMODE;}
<STRMODE>[^\"\\\n]+	{append_text();}
<STRMODE>\"		{yylval.string = end_string();
			 BEGIN INITIAL; return STRING;}

 /* STRING and CHARLIT: these escape sequences are common between the two */
<CHRMODE,STRMODE>{SIMESC}	{/* these are the same for chars/strings */
				 parse_append_escape();}
<CHRMODE,STRMODE>{OCTESC}	{parse_append_octal();}
<CHRMODE,STRMODE>{HEXESC}	{parse_append_hexadecimal();}

 /* STRING and CHARLIT errors */
<STRMODE>.		{print_lexical_error();}
<CHRMODE>.		{print_lexical_error();}
<STRMODE>\n		{print_lexical_error();++lineno;}
<CHRMODE>\n		{print_lexical_error();++lineno;}

 /* */
->		{return INDSEL;}
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
\.\.\.		{return ELLIPSIS;}
\*=		{return TIMESEQ;}
\/=		{return DIVEQ;}
%=		{return MODEQ;}
\+=		{return PLUSEQ;}
-=		{return MINUSEQ;}
\<\<=		{return SHLEQ;}
>>=		{return SHREQ;}
&=		{return ANDEQ;}
\|=		{return OREQ;}
\^=		{return XOREQ;}
auto		{return AUTO;}
break		{return BREAK;}
case		{return CASE;}
char		{return CHAR;}
const		{return CONST;}
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
_Bool		{return _BOOL;}
_Complex	{return _COMPLEX;}
_Imaginary	{return _IMAGINARY;}

 /* IDENT */
[a-zA-Z_][a-zA-Z0-9_]*	{yylval.ident = yytext; return IDENT;}

 /* single-character operators get their ASCII value passed as token type */
[~!%\^&\*\(\)\-\=\+\[\]\{\}\|\;:\<\>,\.\?/] {return yytext[0];}

 /* ignore whitespace (except newline, see below) */
[ \t\v\f\r]+	{;}

 /* newline isn't counted as a token but it updates line counter */
[\n]		{++lineno;}

 /* invalid characters */
.		{print_lexical_error();}
%%

// int main() {
// 	int t;
// 	char *buf = NULL, chr_buf[5];

// 	while (t = yylex()) {

// 		fprintf(stdout, "%s %d ", filename, lineno);

// 		// single character token
// 		if (t < 0xFF) {
// 			fprintf(stdout, "%c \n", t);
// 			continue;
// 		} else {
// 			fprintf(stdout, "%s ", toktostr(t));
// 		}

// 		// special token types
// 		switch (t) {
// 			case NUMBER:
// 				buf = print_number(yylval.num);
// 				break;
// 			case IDENT:
// 				fprintf(stdout, "%s", yylval.ident);
// 				break;
// 			case STRING:
// 				buf = print_string(&yylval.string);
// 				destroy_string(&yylval.string);
// 				break;
// 			case CHARLIT:
// 				// only single-width (ASCII) characters are
// 				// used in test cases
// 				print_char(yylval.charlit.value.none, chr_buf);
// 				fprintf(stdout, "%s", chr_buf);
// 				break;
// 		}

// 		// print_number and print_string dynamically allocate a buffer
// 		if (buf) {
// 			fprintf(stdout, "%s", buf);
// 			free(buf);
// 			buf = NULL;
// 		}
// 		fprintf(stdout, "\n");
// 	}

// 	return 0;
// }