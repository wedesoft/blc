%{
#include <stdio.h>
#include "lambda_bison.h"
extern int yyretval;
%}

%option noyywrap

%%

"0"            { return ZERO; }
"1"            { return ONE; }
"->"|"λ"       { return LAMBDA; }
"("            { return LP; }
")"            { return RP; }
"."            { return DOT; }
[a-z]+         { strcpy(yylval.sym, yytext); return VAR; }
<<EOF>>        { return 0; }
[ \t\n]+       {}
.              {}

%%

