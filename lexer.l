%option noyywrap
%x STRMODE
%x CHRMODE
%{
#include <stdlib.h>
#include <stddef.h>
#include "tokensmanual.h"
#include "stringutils.h"
#include "errorutils.h"
#include "numutils.h"



// TODO: choose appropriate types
typedef union {
	struct number num;
	char c;
	char *ident;
	struct string string;
	struct charlit charlit;
} YYSTYPE;

YYSTYPE yylval;
%}

DEC		[1-9]+[0-9]*
EXP 		[eE][+-]?{DEC}+
OCT 		0[0-7]*
HEX 		0[xX][0-9A-Fa-f]+
P 		[pP][+-]?{DEC}+
REAL 		[0-9]*\.[0-9]*

SIMESC		\\['"\?\\abfnrtv]
OCTESC		\\[0-7]{1,3}	
HEXESC		\\x[0-9a-fA-F]+

%%
 /* Reading a preprocessor line number */
#[ ]{DEC}[ ]\"[^\"]+\".*\n	{parse_lineno();}

 /* NUMBER */
 /*Integer*/
{DEC}		{yylval.num.int_val=strtoull(yytext,NULL,10); yylval.num.sign=SIGNED_T; yylval.num.type=INT_T; return NUMBER;}
{DEC}[Uu]	{yylval.num.int_val=strtoull(yytext,NULL,10); yylval.num.sign=UNSIGNED_T; yylval.num.type = INT_T; return NUMBER;}
{DEC}[Ll]	{yylval.num.int_val=strtoull(yytext,NULL,10); yylval.num.sign=SIGNED_T; yylval.num.type = LONG_T; return NUMBER;}
{DEC}([Ll][Uu]|[Uu][Ll])	{yylval.num.int_val=strtoull(yytext,NULL,10); yylval.num.sign=UNSIGNED_T; yylval.num.type = LONG_T;return NUMBER;}
{DEC}[Ll][Ll]	{yylval.num.int_val=strtoull(yytext,NULL,10); yylval.num.sign=SIGNED_T; yylval.num.type = LONGLONG_T; return NUMBER;}
{DEC}([Uu](LL|ll)|(LL|ll)[Uu])	{yylval.num.int_val=strtoull(yytext,NULL,10); yylval.num.sign=UNSIGNED_T; yylval.num.type = LONGLONG_T; return NUMBER;}

{OCT}		{yylval.num.int_val=strtoull(yytext,NULL,8); yylval.num.sign=SIGNED_T; yylval.num.type=INT_T; return NUMBER;}
{OCT}[Uu]	{yylval.num.int_val=strtoull(yytext,NULL,8); yylval.num.sign=UNSIGNED_T; yylval.num.type = INT_T; return NUMBER;}
{OCT}[Ll]	{yylval.num.int_val=strtoull(yytext,NULL,8); yylval.num.sign=SIGNED_T; yylval.num.type = LONG_T; return NUMBER;}
{OCT}([Ll][Uu]|[Uu][Ll])	{yylval.num.int_val=strtoull(yytext,NULL,8); yylval.num.sign=UNSIGNED_T; yylval.num.type = LONG_T;return NUMBER;}
{OCT}[Ll][Ll]	{yylval.num.int_val=strtoull(yytext,NULL,8); yylval.num.sign=SIGNED_T; yylval.num.type = LONGLONG_T; return NUMBER;}
{OCT}([Uu](LL|ll)|(LL|ll)[Uu])	{yylval.num.int_val=strtoull(yytext,NULL,8); yylval.num.sign=UNSIGNED_T; yylval.num.type = LONGLONG_T; return NUMBER;}


{HEX}		{yylval.num.int_val=strtoull(yytext,NULL,16); yylval.num.sign=SIGNED_T; yylval.num.type=INT_T; return NUMBER;}
{HEX}[Uu]	{yylval.num.int_val=strtoull(yytext,NULL,16); yylval.num.sign=UNSIGNED_T; yylval.num.type = INT_T; return NUMBER;}
{HEX}[Ll]	{yylval.num.int_val=strtoull(yytext,NULL,16); yylval.num.sign=SIGNED_T; yylval.num.type = LONG_T; return NUMBER;}
{HEX}([Ll][Uu]|[Uu][Ll])	{yylval.num.int_val=strtoull(yytext,NULL,16); yylval.num.sign=UNSIGNED_T; yylval.num.type = LONG_T;return NUMBER;}
{HEX}[Ll][Ll]	{yylval.num.int_val=strtoull(yytext,NULL,16); yylval.num.sign=SIGNED_T; yylval.num.type = LONGLONG_T; return NUMBER;}
{HEX}([Uu](LL|ll)|(LL|ll)[Uu])	{yylval.num.int_val=strtoull(yytext,NULL,16); yylval.num.sign=UNSIGNED_T; yylval.num.type = LONGLONG_T; return NUMBER;}

 /*Float*/
{REAL}		{yylval.num.real_val=strtod(yytext,NULL); yylval.num.sign=SIGNED_T; yylval.num.type = DOUBLE_T; return NUMBER;}
{REAL}[Ll] 	{yylval.num.real_val=strtod(yytext,NULL); yylval.num.sign=SIGNED_T; yylval.num.type = LONGDOUBLE_T; return NUMBER;}
{REAL}[Ff]	{yylval.num.real_val=strtof(yytext,NULL); yylval.num.sign=SIGNED_T; yylval.num.type = FLOAT_T; return NUMBER;}
{HEX}{P}	{yylval.num.real_val=strtod(yytext,NULL); yylval.num.sign=SIGNED_T; yylval.num.type = DOUBLE_T; return NUMBER;}
[0]?{DEC}+{EXP}[Ff]?	{char temp = yytext[strlen(yytext)-1];
						yylval.num.real_val=strtod(yytext,NULL); yylval.num.sign=SIGNED_T; 
						if(temp=='F'||temp=='f') yylval.num.type = FLOAT_T; 
						else yylval.num.type = DOUBLE_T;
						return NUMBER;}

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
<CHRMODE>{SIMESC}	{/* these are the same for chars/strings */
			 parse_append_escape();}
<CHRMODE>{OCTESC}	{parse_append_octal();}
<CHRMODE>{HEXESC}	{parse_append_hexadecimal();}
<STRMODE>{SIMESC}	{parse_append_escape();}
<STRMODE>{OCTESC}	{parse_append_octal();}
<STRMODE>{HEXESC}	{parse_append_hexadecimal();}

 /* STRING and CHARLIT errors */
<STRMODE>.		{print_lexical_error();}
<CHRMODE>.		{print_lexical_error();}
<STRMODE>\n		{print_lexical_error();++lineno;}
<CHRMODE>\n		{print_lexical_error();++lineno;}

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

 /* IDENT */
[a-zA-Z_][a-zA-Z0-9_]*	{yylval.ident = yytext; return IDENT;}

 /* single-character operators get their ASCII value passed as token type */
[!#%\^&\*\(\)\-\=\+\[\]\{\}\|\;\<\>\.\?] {return yytext[0];}

 /* ignore whitespace (except newline, see below) */
[ \t\v\f\r]+	{;}

 /* newline isn't counted as a token but it updates line counter */
[\n]		{++lineno;}

 /* invalid characters */
.		{print_lexical_error();}
%%

int main() {
	int t;

	while (t = yylex()) {

		fprintf(stdout, "%s\t%d\t", filename, lineno);

		switch (t) {
			case NUMBER:
				fprintf(stdout, "NUMBER\t");
				print_number(yylval.num);
				break;
			case IDENT:
				fprintf(stdout, "IDENT\t%s\n", yylval.ident);
				break;
			case STRING:
				fprintf(stdout, "STRING\t");
				print_string(&yylval.string);
				fprintf(stdout, "\n");
				destroy_string(&yylval.string);
				break;
			case CHARLIT:
				fprintf(stdout, "CHARLIT (width %d)\t\\x%x\n",
					char_widths[yylval.charlit.width],
					*((unsigned long *) &yylval.charlit.value));
				break;
		}
	}

	return 0;
}