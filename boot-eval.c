/* Bracket - Attempt at writing a small Racket (Scheme) interpreter
 * Copyright (C) 2012  Jan Wedekind
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

#define MAX_CELLS 65536
#define NIL -1

typedef enum { PAIR, TOKEN } type_t;

typedef struct {
  int first;
  int rest;
} pair_t;

typedef char token_t[TOKENSIZE + 3];

typedef struct {
  type_t type;
  union {
    pair_t pair;
    token_t token;
  };
} cell_t;

int n_cells = 0;
cell_t cells[MAX_CELLS];
int environment = NIL;

int add_cell(void)
{
  int retval = n_cells++;
  if (n_cells >= MAX_CELLS) {
    fprintf(stderr, "Error: Program requires more than %d cells\n", MAX_CELLS);
    exit(1);
  };
  return retval;
}

int to_token(const char *str)
{
  int retval = add_cell();
  cells[retval].type = TOKEN;
  if (strlen(str) > TOKENSIZE) {
    fprintf(stderr, "Error: Token %s... longer than %d characters\n", str, TOKENSIZE);
    exit(1);
  };
  strcpy(cells[retval].token, str);
  return retval;
}

int is_nil(int i)
{
  return i == NIL;
}

int is_pair(int i)
{
  return is_nil(i) ? 0 : cells[i].type == PAIR;
}

int is_token(int i)
{
  return is_nil(i) ? 0 : cells[i].type == TOKEN;
}

void print_expression(int i, FILE *stream);

char *token(int i)
{
  if (!is_token(i)) {
    print_expression(i, stderr); fputs(" is not a token\n", stderr);
    exit(1);
  };
  return cells[i].token;
}

int first(int i)
{
  return is_pair(i) ? cells[i].pair.first : NIL;
}

int rest(int i)
{
  return is_pair(i) ? cells[i].pair.rest : NIL;
}

int arg(int i, int n)
{
  return n == 0 ? first(i) : arg(rest(i), n - 1);
}

int cons(int first, int rest)
{
  int retval = add_cell();
  cells[retval].type = PAIR;
  cells[retval].pair.first = first;
  cells[retval].pair.rest = rest;
  return retval;
}

int lambda(int arg, int body)
{
  return cons(to_token("lambda"), cons(arg, cons(body, NIL)));
}

int quote(int i)
{
  return cons(to_token("quote"), cons(i, NIL));
}

int if_(int cond, int a, int b)
{
  return cons(to_token("if"), cons(cond, cons(a, cons(b, NIL))));
}

int cond(int i)
{
  return cons(to_token("cond"), i);
}

int procedure(int arg, int body, int env)
{
  return cons(to_token("#<procedure>"), cons(arg, cons(body, cons(env, NIL))));
}

int lookup(int i, int env);

int to_bool(int value, int env)
{
  int retval;
  if (value)
    retval = first(lookup(to_token("#t"), env));
  else
    retval = first(lookup(to_token("#f"), env));
  return retval;
}

int is_eq(int i, const char *str)
{
  return is_token(i) ? !strcmp(token(i), str) : 0;
}

int eq(int a, int b)
{
  return is_token(b) ? is_eq(a, token(b)) : 0;
}

int lookup(int i, int env)
{
  int retval;
  if (is_nil(env))
    retval = NIL;
  else if (is_eq(first(first(env)), token(i)))
    retval = rest(first(env));
  else
    retval = lookup(i, rest(env));
  return retval;
}

int is_push(int i)
{
  return is_eq(i, "(");
}

int is_pop(int i)
{
  return is_eq(i, ")");
}

int is_procedure(int i)
{
  return is_eq(first(i), "#<procedure>");
}

int is_eval(int i)
{
  return is_eq(first(i), "eval");
}

int read_expression(FILE *stream);

int read_list(FILE *stream)
{
  int retval;
  int cell = read_expression(stream);
  if (is_pop(cell))
    retval = NIL;
  else
    retval = cons(cell, read_list(stream));
  return retval;
}

int read_expression(FILE *stream)
{
  int retval;
  char buffer[TOKENSIZE + 2];
  char *token = read_token(buffer, stream);
  int cell = token ? to_token(token) : NIL;
  if (is_push(cell))
    retval = read_list(stream);
  else
    retval = cell;
  return retval;
}

void print_expression(int i, FILE *stream)
{
  if (is_nil(i))
    fputs("()", stream);
  else if (is_pair(i)) {
    fputc('(', stream);
    print_expression(first(i), stream);
    int c = 1;
    int r = rest(i);
    while (!is_nil(r)) {
      fputc(' ', stream);
      if ((is_procedure(i) && c == 3) || (is_eval(i) && c == 2))
        fputs("#<env>", stream);
      else
        print_expression(first(r), stream);
      r = rest(r);
      c++;
    }
    fputc(')', stream);
  } else
    fputs(token(i), stream);
}

void print_quoted(int i, FILE *stream)
{
  if (is_procedure(i))
    fputs("#<procedure>\n", stream);
  else {
    fputs("(quote ", stream);
    print_expression(i, stream);
    fputs(")\n", stream);
  };
}

int define(int id, int body, int env)
{
  return cons(cons(id, cons(body, NIL)), env);
}

int define_list(int ids, int bodies, int env)
{
  int retval;
  if (is_nil(ids))
    retval = env;
  else
    retval = define_list(rest(ids), rest(bodies), define(first(ids), first(bodies), env));
  return retval;
}

int eval(int i, int env)
{
  return cons(to_token("eval"), cons(i, cons(env, NIL)));
}

int eval_list(int i, int env)
{
  return is_nil(i) ? NIL : cons(eval(first(i), env), eval_list(rest(i), env));
}

int eval_expression(int i, int env);

int eval_each(int i, int env)
{
  return is_nil(i) ? NIL : cons(eval_expression(first(i), env), eval_each(rest(i), env));
}

#ifndef NDEBUG
int maxdepth = 30;
#endif

int eval_expression(int i, int env)
{
  int retval;
#ifndef NDEBUG
  // print_expression(i, stderr); fputs(" ...\n", stderr);
  if (maxdepth <= 0) return to_token("#<recursion>");
  maxdepth -= 1;
#endif
  if (is_nil(i))
    retval = i;
  else if (is_pair(i)) {
    int head = eval_expression(first(i), env);
    if (is_nil(head))
      retval = cons(head, eval_each(rest(i), env));
    else if (is_pair(head)) {
      if (is_procedure(head)) {
        int local_env = arg(head, 3);
        int vars = arg(head, 1);
        int args = eval_list(rest(i), env);
        if (is_token(vars))
          local_env = define(vars, args, local_env);
        else
          local_env = define_list(vars, args, local_env);
        retval = eval_expression(arg(head, 2), local_env);
      } else
        retval = eval_expression(cons(head, rest(i)), env);
    } else {
      if (is_eq(head, "quote"))
        retval = arg(i, 1);
      else if (is_eq(head, "null?"))
        retval = to_bool(is_nil(eval_expression(arg(i, 1), env)), env);
      else if (is_eq(head, "pair?"))
        retval = to_bool(is_pair(eval_expression(arg(i, 1), env)), env);
      else if (is_eq(head, "first"))
        retval = first(eval_expression(arg(i, 1), env));
      else if (is_eq(head, "rest"))
        retval = rest(eval_expression(arg(i, 1), env));
      else if (is_eq(head, "cons"))
        retval = cons(eval_expression(arg(i, 1), env),
                      eval_expression(arg(i, 2), env));
      else if (is_eq(head, "define")) {
        if (environment != env) {
          fputs("define: not allowed in an expression context\n", stderr);
          exit(1);
        };
        environment = define(arg(i, 1), arg(i, 2), environment);
        retval = eval_expression(arg(i, 1), environment);
      } else if (is_eq(head, "lambda"))
        retval = procedure(arg(i, 1), arg(i, 2), env);
      else if (is_eq(head, "eval"))
        retval = eval_expression(arg(i, 1), arg(i, 2));
      else if (is_eq(head, "eq?"))
        retval = to_bool(eq(eval_expression(arg(i, 1), env), eval_expression(arg(i, 2), env)), env);
      else if (is_eq(head, "if")) {
        int condition = arg(i, 1);
        int a = quote(arg(i, 2));
        int b = quote(arg(i, 3));
        retval = eval_expression(eval_expression(cons(condition, cons(a, cons(b, NIL))), env), env);
      } else if (is_eq(head, "cond")) {
        if (is_nil(arg(i, 1)))
          retval = NIL;
        else {
          int rule = arg(i, 1);
          retval = eval_expression(if_(arg(rule, 0), arg(rule, 1), cond(rest(rest(i)))), env);
        };
      } else if (!is_nil(lookup(head, env)))
        retval = eval_expression(cons(first(lookup(head, env)), rest(i)), env);
      else if (is_procedure(i))
        retval = i;
      else
        retval = cons(head, eval_each(rest(i), env));
    }
  } else if (is_eq(i, "null"))
    retval = NIL;
  else if (!is_nil(lookup(i, env)))
    retval = eval_expression(first(lookup(i, env)), env);
  else
    retval = i;
#ifndef NDEBUG
  maxdepth += 1;
  // print_expression(i, stderr); fputs("\n  -> ", stderr);
  // print_expression(retval, stderr); fputc('\n', stderr);
#endif
  return retval;
}

void initialize(void)
{
  FILE *boot = fopen("boot.rkt", "r");
  while (!feof(boot)) eval_expression(read_expression(boot), environment);
  fclose(boot);
}

int main(void)
{
  initialize();
  while (1) {
#ifndef NDEBUG
    // fputs("----------------------------------------\n", stderr);
#endif
    int expr = read_expression(stdin);
    if (feof(stdin)) break;
#if 0
    int i;
    for (i=0; i<n_cells; i++) {
      if (i == expr)
        fprintf(stderr, "-> ");
      else
        fprintf(stderr, "   ");
      fprintf(stderr, "%2d: ", i);
      if (is_pair(i))
        fprintf(stderr, "first = %3d, rest = %3d - ", first(i), rest(i));
      else
        fprintf(stderr, "token = %15s - ", token(i));
      print_expression(i, stderr);
      fputc('\n', stderr);
    };
#endif
#if 0
    print_expression(expr, stderr);
    fputc('\n', stderr);
#endif
    print_quoted(eval_expression(expr, environment), stdout);
#if 0
    fputc('\n', stderr);
#endif
  };
  return 0;
}

