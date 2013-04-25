%{
#include <stdio.h>
#include "lambda_bison.h"
extern int yyretval;
%}

%option noyywrap

%%

"->"|"λ"      { return LAMBDA; }
[a-z][a-z0-9] { yylval.sym = 0; return SYM; }
<<EOF>>       { return 0; }
[ \t\n]+      {}
.             { fprintf(stderr, "Unrecognized token: \"%s\"\n", yytext); yyretval = 1; }
%%

