/* BLC - Binary Lambda Calculus VM
 * Copyright (C) 2013  Jan Wedekind
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blc.h"

extern int yyretval;
extern FILE *yyin;
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

char *name[STACKSIZE];
int names;
char buffer[NAMEBUFSIZE];
char *buffer_p;

void push(const char *token)
{
  name[names++] = buffer_p;
  strcpy(buffer_p, token);
  buffer_p += strlen(token) + 1;
}

void pop(void)
{
  if (names > 0) {
    buffer_p = name[names - 1];
    names--;
  } else
    buffer_p = buffer;
}

int find_var(const char *token)
{
  int retval = -1;
  char *p = buffer;
  while (p < buffer_p) {
    if (retval >= 0) retval++;
    if (!strcmp(p, token)) retval = 0;
    p += strlen(p) + 1;
  }
  if (retval == -1) {
    yyerror("Unknown variable ");
    yyerror(token);
    exit(1);
  };
  return retval;
}

int n_definitions;
int n_prev;
int previous_expr;

int copy_definitions(int previous_expr, int expression, int n)
{
  int retval;
  if (n == 0)
    retval = expression;
  else {
    int subexpr = copy_definitions(body(fun(previous_expr)),
                                   expression,
                                   n - 1);
    retval = call(lambda(subexpr), arg(previous_expr));
  };
  return retval;
}

%}

%union {
  int expr;
  int var;
  char sym[80];
};

%type <expr> expr subexpr nodef lambda call abstraction blc blc_var blc_lambda blc_call
%token <sym> VAR_ DEF
%token ZERO ONE LAMBDA_ LP RP DOT

%%
init: { n_definitions = 0;
        n_prev = 0;
        init();
        previous_expr = f();
        push("input"); } run { pop(); destroy(); }
    ;

run: /* empty */
   | expr { int expression = copy_definitions(previous_expr, $1, n_prev);
            int environment = pair(bytes_to_bits(select_binary(input(yyin))), f());
            write_expression(expression, environment, yyout);
            previous_expr = expression;
            n_prev = n_definitions; } run
   ;

expr: VAR_       { $$ = var(find_var($1)); }
    | lambda
    | LP call RP { $$ = $2; }
    | DEF nodef  { push($1); } expr { $$ = call(lambda($4), $2); n_definitions++; }
    | blc
    ;

blc: blc_var
   | blc_lambda
   | blc_call
   ;

blc_var: ONE ZERO    { $$ = var(0); }
       | ONE blc_var { $$ = var(idx($2) + 1); }
       ;

blc_lambda: ZERO ZERO blc { $$ = lambda($3); }
          ;

blc_call: ZERO ONE blc blc { $$ = call($3, $4); }
        ;

call: subexpr
    | call subexpr { $$ = call($1, $2); }
    ;

subexpr: VAR_       { $$ = var(find_var($1)); }
       | lambda
       | LP call RP { $$ = $2; }
       | DEF nodef  { push($1); } subexpr { $$ = call(lambda($4), $2); pop(); }
       ;

nodef: VAR_          { $$ = var(find_var($1)); }
     | lambda
     | LP call RP    { $$ = $2; }
     ;

lambda: LAMBDA_ DOT         { push(""); } subexpr { $$ = lambda($4); pop(); }
      | LAMBDA_ abstraction { $$ = $2; }
      | LAMBDA_             { push(""); } lambda  { $$ = lambda($3); pop(); }
      | LAMBDA_ VAR_        { push($2); } lambda  { $$ = lambda($4); pop(); }
      ;

abstraction: VAR_ { push($1); } DOT subexpr { $$ = lambda($4); pop(); }
           | VAR_ { push($1); } abstraction { $$ = lambda($3); pop(); }
           ;

%%
