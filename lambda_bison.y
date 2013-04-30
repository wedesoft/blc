%{
#include <stdio.h>
#include <string.h>
#include "blc.h"

extern int yyretval;
extern FILE *yyout;
extern char *yytext;
extern int n_registers;

int yylex();
int yyerror(const char *p)
{
  fprintf(stderr, "Error: %s\n", p);
  yyretval = 1;
  return yyretval;
}

#define STACKSIZE  16384
#define NAMEBUFSIZE 65536

char *stack[STACKSIZE];
int stack_n;
char names[NAMEBUFSIZE];
char *name_p;

void push(void)
{
  stack[stack_n++] = name_p;
}

void pop(void)
{
  if (stack_n > 0) {
    name_p = stack[stack_n - 1];
    stack_n--;
  } else
    name_p = names;
}

int find_var(const char *token)
{
  int retval = -1;
  char *p = names;
  while (p < name_p) {
    if (retval >= 0)
      retval++;
    else if (!strcmp(p, token))
      retval = 0;
    p += strlen(p) + 1;
  }
  return retval;
}
%}

%union {
  int expr;
  int var;
  char sym[16];
};

%type <expr> expr lambda call
%type <var> variable
%token <sym> VAR
%token ZERO ONE LAMBDA LP RP DOT

%%
run: /* empty */
   | expr { print_expression($1, yyout); gc_pop(n_registers); } run
   ;

expr: variable    { $$ = gc_push(make_variable($1)); }
    | VAR         { $$ = gc_push(make_variable(find_var($1))); }
    | lambda expr { $$ = gc_push(make_lambda($2)); pop(); }
    | call expr   { $$ = gc_push(make_call($1, $2)); }
    ;
   
variable: ONE ZERO     { $$ = 0; }
        | ONE variable { $$ = $2 + 1; }
        ;

lambda: ZERO ZERO      { push(); *name_p = '\0'; name_p += 1; }
      | ZERO ZERO DOT  { push(); *name_p = '\0'; name_p += 1; }
      | LAMBDA VAR     { push(); strcpy(name_p, $2); name_p += strlen($2) + 1; }
      | LAMBDA VAR DOT { push(); strcpy(name_p, $2); name_p += strlen($2) + 1; }
      | LAMBDA         { push(); *name_p = '\0'; name_p += 1; }
      | LAMBDA DOT     { push(); *name_p = '\0'; name_p += 1; }
      ;

call: ZERO ONE expr { $$ = $3; }
    | LP expr RP    { $$ = $2; }

%%
