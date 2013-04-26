%{
#include <stdio.h>
#include "blc.h"

extern int yyretval;
extern FILE *yyout;
extern int n_registers;

int yylex();
int yyerror(const char *p)
{
  fprintf(stderr, "Error: %s\n", p);
  yyretval = 1;
}
%}

%union {
  int expr;
  int var;
};

%type <expr> expr lambda call
%type <var> variable
%token ZERO ONE LAMBDA LP RP
%token <var> VAR

%%
run: /* empty */
   | expr run { print_expression($1, yyout); gc_pop(n_registers); }
   ;

expr: variable { $$ = gc_push(make_variable($1)); }
    | lambda
    | call
    ;
   
variable: ONE ZERO     { $$ = 0; }
        | ONE variable { $$ = $2 + 1; }
        ;

lambda: ZERO ZERO expr { $$ = gc_push(make_lambda($3)); }
      | LAMBDA expr    { $$ = gc_push(make_lambda($2)); }
      ;

call: ZERO ONE expr expr { $$ = gc_push(make_call($3, $4)); }
    | LP expr expr RP    { $$ = gc_push(make_call($2, $3)); }

%%
