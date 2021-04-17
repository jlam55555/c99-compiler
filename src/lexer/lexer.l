%option noyywrap
%x STRMODE
%x CHRMODE
%{
#include <stdlib.h>
#include <stddef.h>
#include <lexer/stringutils.h>
#include <lexer/errorutils.h>
#include <lexer/numutils.h>
#include <parser.tab.h>

//YYSTYPE yylval;

// helper for making integral and float types
#define MAKEINT(radix, type, lls, sign) \
	yylval.astnode = make_int(radix, type, lls, sign);\
	return NUMBER;
#define MAKEFP(type, lls) \
	yylval.astnode = make_fp(type, lls);\
	return NUMBER;
#define SC(x) \
	yylval.sc = x;\
	return x
%}
%option yylineno

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
{DEC}				{MAKEINT(10,BT_INT,LLS_UNSPEC,SIGN_SIGNED);}
{DEC}[Uu]			{MAKEINT(10,BT_INT,LLS_UNSPEC,SIGN_UNSIGNED);}
{DEC}[Ll]			{MAKEINT(10,BT_INT,LLS_LONG,SIGN_SIGNED);}
{DEC}{UL}			{MAKEINT(10,BT_INT,LLS_LONG,SIGN_UNSIGNED);}
{DEC}{LL}			{MAKEINT(10,BT_INT,LLS_LONG_LONG,SIGN_SIGNED);}
{DEC}{ULL}			{MAKEINT(10,BT_INT,LLS_LONG_LONG,SIGN_UNSIGNED);}

{OCT}				{MAKEINT(8,BT_INT,LLS_UNSPEC,SIGN_SIGNED);}
{OCT}[Uu]			{MAKEINT(8,BT_INT,LLS_UNSPEC,SIGN_UNSIGNED);}
{OCT}[Ll]			{MAKEINT(8,BT_INT,LLS_LONG,SIGN_SIGNED);}
{OCT}{UL}			{MAKEINT(8,BT_INT,LLS_LONG,SIGN_UNSIGNED);}
{OCT}{LL}			{MAKEINT(8,BT_INT,LLS_LONG_LONG,SIGN_SIGNED);}
{OCT}{ULL}			{MAKEINT(8,BT_INT,LLS_LONG_LONG,SIGN_UNSIGNED);}

{HEX}				{MAKEINT(16,BT_INT,LLS_UNSPEC,SIGN_SIGNED);}
{HEX}[Uu]			{MAKEINT(16,BT_INT,LLS_UNSPEC,SIGN_UNSIGNED);}
{HEX}[Ll]			{MAKEINT(16,BT_INT,LLS_LONG,SIGN_SIGNED);}
{HEX}{UL}			{MAKEINT(16,BT_INT,LLS_LONG,SIGN_UNSIGNED);}
{HEX}{LL}			{MAKEINT(16,BT_INT,LLS_LONG_LONG,SIGN_SIGNED);}
{HEX}{ULL}			{MAKEINT(16,BT_INT,LLS_LONG_LONG,SIGN_UNSIGNED);}

 /* Float */
{REAL}				{MAKEFP(BT_DOUBLE,LLS_UNSPEC);}
{REAL}[Ll] 			{MAKEFP(BT_DOUBLE,LLS_LONG);}
{REAL}[Ff]			{MAKEFP(BT_FLOAT,LLS_UNSPEC);}
{HEX}{P}			{MAKEFP(BT_DOUBLE,LLS_UNSPEC);}
[0]?{DEC}+{EXP}[Ff]?		{char tmp=yytext[strlen(yytext)-1];
				 MAKEFP((tmp|32)=='f'?BT_FLOAT:BT_DOUBLE,
				 	LLS_UNSPEC);}
[0]?{DEC}+{EXP}[Ll]?		{char tmp=yytext[strlen(yytext)-1];
				 MAKEFP(BT_DOUBLE,
				 	(tmp|32)=='l'?LLS_LONG:LLS_UNSPEC);}

 /* STRING and CHARLIT; these differ between the two */
[uUL]?\'			{begin_literal(); BEGIN CHRMODE;}
<CHRMODE>[^\'\\\n]+		{append_text();}
<CHRMODE>\'			{yylval.charlit = end_charlit();
				 BEGIN INITIAL; return CHARLIT;}
(u8|[uUL])?\"			{begin_literal(); BEGIN STRMODE;}
<STRMODE>[^\"\\\n]+		{append_text();}
<STRMODE>\"			{yylval.string = end_string();
				 BEGIN INITIAL; return STRING;}

 /* STRING and CHARLIT: these escape sequences are common between the two */
<CHRMODE,STRMODE>{SIMESC}	{/* these are the same for chars/strings */
				 parse_append_escape();}
<CHRMODE,STRMODE>{OCTESC}	{parse_append_octal();}
<CHRMODE,STRMODE>{HEXESC}	{parse_append_hexadecimal();}

 /* STRING and CHARLIT errors */
<STRMODE>.			{print_lexical_error();}
<CHRMODE>.			{print_lexical_error();}
<STRMODE>\n			{print_lexical_error();++lineno;}
<CHRMODE>\n			{print_lexical_error();++lineno;}

 /* */
->				{SC(INDSEL);}
\+\+				{SC(PLUSPLUS);}
--				{SC(MINUSMINUS);}
\<\<				{SC(SHL);}
>>				{SC(SHR);}
\<=				{SC(LTEQ);}
>=				{SC(GTEQ);}
==				{SC(EQEQ);}
!=				{SC(NOTEQ);}
&&				{SC(LOGAND);}
\|\|				{SC(LOGOR);}
\.\.\.				{SC(ELLIPSIS);}
\*=				{SC(TIMESEQ);}
\/=				{SC(DIVEQ);}
%=				{SC(MODEQ);}
\+=				{SC(PLUSEQ);}
-=				{SC(MINUSEQ);}
\<\<=				{SC(SHLEQ);}
>>=				{SC(SHREQ);}
&=				{SC(ANDEQ);}
\|=				{SC(OREQ);}
\^=				{SC(XOREQ);}
auto				{SC(AUTO);}
break				{SC(BREAK);}
case				{SC(CASE);}
char				{SC(CHAR);}
const				{SC(CONST);}
continue			{SC(CONTINUE);}
default				{SC(DEFAULT);}
do				{SC(DO);}
double				{SC(DOUBLE);}
else				{SC(ELSE);}
enum				{SC(ENUM);}
extern				{SC(EXTERN);}
float				{SC(FLOAT);}
for				{SC(FOR);}
goto				{SC(GOTO);}
if				{SC(IF);}
inline				{SC(INLINE);}
int				{SC(INT);}
long				{SC(LONG);}
register			{SC(REGISTER);}
restrict			{SC(RESTRICT);}
return				{SC(RETURN);}
short				{SC(SHORT);}
signed				{SC(SIGNED);}
sizeof				{SC(SIZEOF);}
static				{SC(STATIC);}
struct				{SC(STRUCT);}
switch				{SC(SWITCH);}
typedef				{SC(TYPEDEF);}
union				{SC(UNION);}
unsigned			{SC(UNSIGNED);}
void				{SC(VOID);}
volatile			{SC(VOLATILE);}
while				{SC(WHILE);}
_Bool				{SC(_BOOL);}
_Complex			{SC(_COMPLEX);}
_Imaginary			{SC(_IMAGINARY);}

 /* IDENT */
[a-zA-Z_][a-zA-Z0-9_]*		{yylval.ident=strdup(yytext);return IDENT;}

 /* single-character operators get their ASCII value passed as token type */
[~!%\^&\*\(\)\-\=\+\[\]\{\}\|\;:\<\>,\.\?/]	{SC(yytext[0]);}

 /* ignore whitespace (except newline, see below) */
[ \t\v\f\r]+			{;}

 /* newline isn't counted as a token but it updates line counter */
[\n]				{++lineno;}

 /* invalid characters */
.				{print_lexical_error();}
%%