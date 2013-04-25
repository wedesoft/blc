%{
#include <stdio.h>
extern int yyretval;
extern FILE *yyout;
int yylex();
int yyerror(const char *p) { fprintf(stderr, "Error: %s\n", p); yyretval = 1; }
%}

%union {
  int sym;
};

%token LAMBDA
%token <sym> SYM

%%
run: LAMBDA { fprintf(yyout, "00"); }

%%
