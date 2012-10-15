/* Bracket - ...
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

#define MAX_CELLS 1024
#define TOKENSIZE 8
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

void print_expression(int i, FILE *stream);

char *token(int i)
{
  if (is_nil(i) || is_pair(i)) {
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

int lookup(int i, int env);

int eq(int a, int b)
{
  int retval;
  if (is_pair(a) || is_pair(b) || is_nil(a) || is_nil(b))
    retval = NIL;
  else if (!strcmp(token(a), token(b)))
    retval = lookup(to_token("true"), environment);
  else
    retval = lookup(to_token("false"), environment);
  return retval;
}

int define(int id, int body)
{
  environment = cons(cons(id, body), environment);
  return body;
}

int undefine(int id)
{
  return define(id, NIL);
}

int push(int i)
{
  int retval;
  if (!is_nil(i) && !is_pair(i))
    retval = token(i)[0] == '(';
  else
    retval = 0;
  return retval;
}

int pop(int i)
{
  int retval;
  if (!is_nil(i) && !is_pair(i))
    retval = token(i)[0] == ')';
  else
    retval = 0;
  return retval;
}

int read_expression(void);

int read_list(void)
{
  int retval;
  int cell = read_expression();
  if (pop(cell))
    retval = NIL;
  else
    retval = cons(cell, read_list());
  return retval;
}

int read_expression(void)
{
  int retval;
  char buffer[TOKENSIZE + 2];
  char *token = read_token(buffer, stdin);
  int cell = token ? to_token(token) : NIL;
  if (push(cell))
    retval = read_list();
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
    int r = rest(i);
    while (!is_nil(r)) {
      fputc(' ', stream);
      print_expression(first(r), stream);
      r = rest(r);
    }
    fputc(')', stream);
  } else
    fputs(token(i), stream);
}

void print_quoted(int i, FILE *stream)
{
  fputs("(quote ", stream);
  print_expression(i, stream);
  fputs(")\n", stream);
}

int eval_expression(int i);

int lookup(int i, int env)
{
  int retval;
  if (is_nil(env))
    retval = NIL;
  else if (!strcmp(token(i), token(first(first(env)))))
    retval = rest(first(env));
  else
    retval = lookup(i, rest(env));
  return retval;
}

int eval_expression(int i)
{
  int retval;
#if 0
  fputs("  ", stderr);
  print_expression(i, stderr);
  fputs(" -> ...\n", stderr);
#endif
  if (is_nil(i))
    retval = i;
  else if (is_pair(i)) {
    if (is_pair(first(i))) {
      int fun = eval_expression(first(i));
      if (!strcmp(token(first(fun)), "lambda")) {
        int backup = environment;
#ifndef NDEBUG
        fputs("define(", stderr);
        print_expression(first(rest(fun)), stderr);
        fputs(", ", stderr);
        print_expression(first(rest(i)), stderr);
        fputs(")\n", stderr);
#endif
        define(first(rest(fun)), first(rest(i)));
        retval = eval_expression(first(rest(rest(fun))));
        environment = backup;
      } else
        retval = cons(fun, rest(i));
    } else {
      char *p = token(first(i));
      if (!strcmp(p, "quote"))
        retval = first(rest(i));
      else if (!strcmp(p, "first"))
        retval = first(eval_expression(first(rest(i))));
      else if (!strcmp(p, "rest"))
        retval = rest(eval_expression(first(rest(i))));
      else if (!strcmp(p, "cons"))
        retval = cons(eval_expression(first(rest(i))),
                      eval_expression(first(rest(rest(i)))));
      else if (!strcmp(p, "define"))
        retval = define(first(rest(i)), eval_expression(first(rest(rest(i)))));
      else if (!strcmp(p, "lambda")) {
        int backup = environment;
        undefine(first(rest(i)));
        retval = lambda(first(rest(i)), eval_expression(first(rest(rest(i)))));
        environment = backup;
      } else if (!strcmp(p, "eq"))
        retval = eq(eval_expression(first(rest(i))), eval_expression(first(rest(rest(i)))));
      else if (!is_nil(lookup(first(i), environment)))
        retval = eval_expression(cons(lookup(first(i), environment), rest(i)));
      else
        retval = cons(first(i), eval_expression(rest(i)));
    }
  } else if (!is_nil(lookup(i, environment)))
    retval = lookup(i, environment);
  else
    retval = i;
#ifndef NDEBUG
  fputs("-> ", stderr);
  print_expression(retval, stderr);
  fputc('\n', stderr);
#endif
  return retval;
}

void initialize(void)
{
  int b = to_token("b");
  int x = to_token("x");
  int y = to_token("y");
  define(to_token("true"), lambda(x, lambda(y, x)));
  define(to_token("false"), lambda(x, lambda(y, y)));
  define(to_token("not"),
         lambda(b, lambda(x, lambda(y, cons(cons(b, cons(y, NIL)), cons(x, NIL))))));
}

//   pair (not atom)
// 1 eq (also compares with nil)
// x car
// x cdr
// x cons
//   null
//
// x define (local environment?)
// x lambda (lambda (arg) (body)), (((lambda (y) (lambda (x) (* x y))) 2) 3)
//   cond
//
//   equal
//   cadr
//   caddr
//   append
//   pair
//   assoc
//   sublis (hash)
//
//   eval
// x quote

int main(void)
{
  initialize();
  while (1) {
    int expr = read_expression();
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
#ifndef NDEBUG
    print_expression(expr, stderr);
    fputc('\n', stderr);
#endif
    print_quoted(eval_expression(expr), stdout);
#ifndef NDEBUG
    fputc('\n', stderr);
#endif
  };
  return 0;
}

