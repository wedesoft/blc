%{
#include <stdio.h>
#include "lambda_bison.h"
extern int yyretval;
%}

%option noyywrap

%%

"0"           { return ZERO; }
"1"           { return ONE; }
"->"|"λ"      { return LAMBDA; }
"("           { return LP; }
")"           { return RP; }
[a-z][a-z0-9] { yylval.var = 0; return VAR; }
<<EOF>>       { return 0; }
[ \t\n]+      {}
.             {}
%%

