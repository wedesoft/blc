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
int stack = NIL;

int add_cell(void)
{
  int retval = n_cells++;
  if (n_cells >= MAX_CELLS) {
    fprintf(stderr, "Error: Program requires more than %d cells\n", MAX_CELLS);
    exit(1);
  };
  return retval;
}

int read_token(void)
{
  int retval = add_cell();
  int len = 0;
  char *p = cells[retval].token;
  cells[retval].type = TOKEN;
  while (1) {
    char c = fgetc(stdin);
    if (c == EOF || c == '\n') break;
    *p++ = c;
    len++;
    if (len > TOKENSIZE) {
      *p = '\0';
      fprintf(stderr,
              "Error: Token %s... longer than %d characters\n",
              cells[retval].token, TOKENSIZE);
      exit(1);
    };
  };
  if (len <= 0) {
    retval = NIL;
#ifndef NDEBUG
    fprintf(stderr, "Reading NIL token\n");
#endif
  } else {
    *p = '\0';
#ifndef NDEBUG
    fprintf(stderr, "Reading token \"%s\"\n", cells[retval].token);
#endif
  }
  return retval;
}

int nil(int i)
{
  return i == NIL;
}

int pair(int i)
{
  return nil(i) ? 0 : cells[i].type == PAIR;
}

char *token(int i)
{
  if (nil(i) || pair(i)) {
    fprintf(stderr, "Not a token\n");
    exit(1);
  };
  return cells[i].token;
}

int first(int i)
{
  return pair(i) ? cells[i].pair.first : NIL;
}

int rest(int i)
{
  return pair(i) ? cells[i].pair.rest : NIL;
}

int cons(int first, int rest)
{
  int retval = add_cell();
  cells[retval].type = PAIR;
  cells[retval].pair.first = first;
  cells[retval].pair.rest = rest;
  return retval;
}

int define(int id, int body)
{
  environment = cons(cons(id, body), environment);
  return body;
}

int push_stack(int arg)
{
  stack = cons(arg, stack);
  return arg;
}

int pop_stack(void)
{
  int retval = first(stack);
  stack = rest(stack);
  return retval;
}

int read_expression(void);

int push(int i)
{
  int retval;
  if (!nil(i) && !pair(i))
    retval = token(i)[0] == '(';
  else
    retval = 0;
  return retval;
}

int pop(int i)
{
  int retval;
  if (!nil(i) && !pair(i))
    retval = token(i)[0] == ')';
  else
    retval = 0;
  return retval;
}

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
  int cell = read_token();
  if (push(cell))
    retval = read_list();
  else
    retval = cell;
  return retval;
}

void print_expression(int i, FILE *stream);

void print_list(int i, FILE *stream)
{
  if (pair(i)) {
    print_expression(first(i), stream);
    if (!nil(rest(i))) {
      fputc(' ', stream);
      print_list(rest(i), stream);
    }
  } else
    print_expression(i, stream);
}

void print_expression(int i, FILE *stream)
{
  if (!nil(i)) {
    if (pair(i)) {
      fputc('(', stream);
      print_list(i, stream);
      fputc(')', stream);
    } else
      fputs(token(i), stream);
  } else {
    fputs("()", stream);
  }
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
  if (nil(env))
    retval = i;
  else if (strcmp(token(i), token(first(first(env)))) == 0)
    retval = rest(first(env));
  else
    retval = lookup(i, rest(env));
  return retval;
}

int eval_each(int i)
{
  int retval;
  if (nil(i))
    retval = i;
  else
    retval = cons(eval_expression(first(i)), eval_each(rest(i)));
  return retval;
}

int eval_list(int i)
{
#ifndef NDEBUG
  print_expression(i, stderr); fputs("\n", stderr);
#endif
  int retval;
  if (nil(i))
    retval = i;
  else if (pair(first(i))) {
    // put value on stack.
    push_stack(first(rest(i)));
    retval = eval_expression(first(i));
  } else {
    char *p = token(first(i));
    if (strcmp(p, "quote") == 0)
      retval = first(rest(i));
    else if (strcmp(p, "first") == 0)
      retval = first(eval_expression(first(rest(i))));
    else if (strcmp(p, "rest") == 0)
      retval = rest(eval_expression(first(rest(i))));
    else if (strcmp(p, "cons") == 0)
      retval = cons(eval_expression(first(rest(i))),
                    eval_expression(first(rest(rest(i)))));
    else if (strcmp(p, "define") == 0)
      retval = define(first(rest(i)),
                      eval_expression(first(rest(rest(i)))));
    else if (strcmp(p, "lambda") == 0) {
      if (nil(stack))
        retval = i;
      else {
        // temporarily replace value.
        int backup = environment;
        define(first(rest(i)), pop_stack());
        retval = eval_expression(first(rest(rest(i))));
        environment = backup;
      }
    } else if (!nil(lookup(first(i), environment))) {
      retval = i; //eval_expression(cons(lookup(first(i), environment), rest(i)));
    } else {
      retval = cons(first(i), eval_each(rest(i)));
    }
  };
  return retval;
}

int eval_expression(int i)
{
  int retval;
  if (nil(i)) {
    retval = i;
  } else if (pair(i)) {
    retval = eval_list(i);
  } else {
#ifndef NDEBUG
    fprintf(stderr, "looking up %s\n", token(i));
#endif
    retval = lookup(i, environment);
  };
  return retval;
}

//   pair (not atom)
//   eq (also compares with nil)
// x car
// x cdr
// x cons
//   null
//
// x define (local environment?)
// 3 lambda (lambda (arg) (body)), (((lambda (y) (lambda (x) (* x y))) 2) 3)
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
  while (1) {
    int expr = read_expression();
    if (feof(stdin)) break;
#ifndef NDEBUG
    int i;
    for (i=0; i<n_cells; i++) {
      if (i == expr)
        fprintf(stderr, "-> ");
      else
        fprintf(stderr, "   ");
      fprintf(stderr, "%2d: ", i);
      if (pair(i))
        fprintf(stderr, "first = %2d, rest = %2d\n", first(i), rest(i));
      else
        fprintf(stderr, "token = %s\n", token(i));
    };
#endif
    // print_quoted(expr, stdout);
    print_quoted(eval_expression(expr), stdout);
#ifndef NDEBUG
    fputc('\n', stderr);
#endif
  };
  return 0;
}

