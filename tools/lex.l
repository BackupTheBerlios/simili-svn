%option noyywrap
%{
#include <stdio.h>
#include "parse.h"
#include "text2midi.h"
//#define YYLVAL_TYPE int
#if 1
#define lexdbg(str)
#else
#define lexdbg(str) printf("lexdbg[%d]: %s\n", __LINE__, str)
#endif
//DATASTR		\"[[:print:]^"\n;]*["\n;^[:print:]]
%}

DATASTR		\"[^"\n]*["\n]
ID		[a-zA-Z_][a-zA-Z0-9_]*
DIGIT		[0-9]+

%%

{DATASTR}		{lexdbg("DATASTR");yylval.datastr=strdup(yytext);return DATASTR_TOKEN;}
[@+][0-9]+		{lexdbg("TIMESTAMP");yylval.digit=text2timestamp(T2M, yytext);return TIMESTAMP_TOKEN;}
\[HEADER\]		{lexdbg("HEADER");return HEADER_TOKEN;}
\[TRACK\]		{lexdbg("TRACK");return TRACK_TOKEN;}
{DIGIT}			{lexdbg("DIGIT");yylval.digit=atoi(yytext);return DIGIT_TOKEN;}
{ID}/([:space:]*=)	{lexdbg("VARIABLE");yylval.id=strdup(yytext);return VARIABLE_TOKEN;}
{ID}			{lexdbg("ID");yylval.id=strdup(yytext);return ID_TOKEN;}
[;,:=]			{lexdbg(yytext);return *yytext;}
[:space:]		{if(*yytext=='\n')line++;}	/* WE EAT THE SPACE */

%%
