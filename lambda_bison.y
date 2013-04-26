%{
#include <stdio.h>
extern int yyretval;
extern FILE *yyout;
int yylex();
int yyerror(const char *p) { fprintf(stderr, "Error: %s\n", p); yyretval = 1; }
%}

%union {
  int sym;
  char c;
};

%token ZERO ONE LAMBDA LP RP
%token <sym> SYM

%%
run: expr

expr: variable
    | lambda
    | call
    ;
   
variable: ONE ZERO     { fprintf(yyout, "10"); }
        | ONE variable { fprintf(yyout, "1"); }
        ;

lambda: ZERO ZERO expr { fprintf(yyout, "00"); }
      | LAMBDA expr    { fprintf(yyout, "00"); }
      ;

call: ZERO ONE expr expr { fprintf(yyout, "01"); }
    | LP expr expr RP    { fprintf(yyout, "01"); }

%%
