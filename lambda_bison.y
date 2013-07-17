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

char *stack[STACKSIZE];
int stack_n;
char names[NAMEBUFSIZE];
char *name_p;

void push(const char *token)
{
  stack[stack_n++] = name_p;
  strcpy(name_p, token);
  name_p += strlen(token) + 1;
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

/* int copy_definitions(int previous_expr, int expression)
{
  int retval;
  gc_push(previous_expr);
  gc_push(expression);
  if (is_definition(previous_expr)) {
    int subexpr = gc_push(copy_definitions(body(previous_expr), expression));
    retval = make_pair(make_definition(term(previous_expr), first(subexpr)), rest(subexpr));
    gc_pop(1);
  } else
    retval = expression;
  gc_pop(2);
  return retval;
}
*/
%}

%union {
  int expr;
  int var;
  char sym[80];
};

%type <expr> expr subexpr nodef lambda call abstraction
%token <sym> VAR DEF
%token ZERO ONE LAMBDA LP RP DOT

%%
init: { push("output"); push("input"); } run { pop(); pop(); }
    ;

run: /* empty */
   | expr { int expression = gc_push($1);
            int input = gc_push(make_input(yyin));
            int output = gc_push(make_output(yyout));
            int environment = gc_push(make_pair(input,
                                                make_pair(output, gc_push(make_false()))));
            int output_rest = gc_push(make_output(yyout));
            write_expression(output_rest, normalise(eval_expression(expression, environment),
                                                    gc_push(make_false()), 0, 2));
            fputc('\n', yyout);
            fflush(yyout);
            gc_pop(n_registers); } run
   | ZERO { fputc('0', yyout); fflush(yyout); } run
   | ONE  { fputc('1', yyout); fflush(yyout); } run
   | DOT  {} run
   ;

expr: VAR        { $$ = gc_push(make_variable(find_var($1))); }
    | lambda     { $$ = $1; }
    | LP call RP { $$ = $2; }
    | DEF nodef  { push($1); } expr { $$ = gc_push(make_call(make_lambda($4), $2)); }
    ;

call: subexpr      { $$ = $1; }
    | call subexpr { $$ = gc_push(make_call($1, $2)); }
    ;

subexpr: VAR        { $$ = gc_push(make_variable(find_var($1))); }
       | lambda     { $$ = $1; }
       | LP call RP { $$ = $2; }
       | DEF nodef  { push($1); } subexpr { $$ = gc_push(make_call(make_lambda($4), $2)); pop(); }
       ;

nodef: VAR           { $$ = gc_push(make_variable(find_var($1))); }
     | lambda        { $$ = $1; }
     | LP call RP    { $$ = $2; }
     ;

lambda: LAMBDA DOT         { push(""); } subexpr { $$ = gc_push(make_lambda($4)); pop(); }
      | LAMBDA abstraction { $$ = $2; }
      | LAMBDA             { push(""); } lambda  { $$ = gc_push(make_lambda($3)); pop(); }
      | LAMBDA VAR         { push($2); } lambda  { $$ = gc_push(make_lambda($4)); pop(); }
      ;

abstraction: VAR { push($1); } DOT subexpr { $$ = gc_push(make_lambda($4)); pop(); }
           | VAR { push($1); } abstraction { $$ = gc_push(make_lambda($3)); pop(); }
           ;

%%
