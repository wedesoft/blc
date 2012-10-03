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

#define MAX_CELLS 128
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
  if (len <= 0)
    retval = NIL;
  else
    *p = '\0';
  return retval;
}

int make_pair(int first, int rest)
{
  int retval = add_cell();
  if (!empty(rest) && !pair(rest)) {
    fprintf(stderr,
            "Error: Rest of pair must be a list (but was %s)\n",
            cells[rest].token);
    exit(1);
  };
  cells[retval].type = PAIR;
  cells[retval].pair.first = first;
  cells[retval].pair.rest = rest;
  return retval;
}

int empty(int i)
{
  return i == NIL;
}

int pair(int i)
{
  return cells[i].type == PAIR;
}

char *token(int i)
{
  if (pair(i)) {
    fprintf(stderr, "Not a token\n");
    exit(1);
  };
  return cells[i].token;
}

int first(int i)
{
  if (!pair(i)) {
    fprintf(stderr, "Argument to 'first' must be a pair\n");
    exit(1);
  };
  return cells[i].pair.first;
}

int rest(int i)
{
  if (!pair(i)) {
    fprintf(stderr, "Argument to 'rest' must be a pair\n");
    exit(1);
  };
  return cells[i].pair.rest;
}

int read_list(void)
{
  int retval;
  int cell = read_expression();
  if (empty(cell))
    retval = NIL;
  else
    retval = make_pair(cell, read_list());
  return retval;
}

int read_expression(void)
{
  int retval = read_token();
  if (!empty(retval)) {
    char *str = token(retval);
    switch (str[0]) {
    case '(':
      retval = read_list();
      break;
    case ')':
      retval = NIL;
      break;
    };
  };
  return retval;
}

void print_expression(int i);

void print_list(int i)
{
  if (pair(i)) {
    print_expression(first(i));
    if (!empty(rest(i))) {
      fputc(' ', stdout);
      print_list(rest(i));
    }
  } else
    print_expression(i);
}

void print_expression(int i)
{
  if (empty(i)) {
    fputs("()", stdout);
  } else if (pair(i)) {
    fputc('(', stdout);
    print_list(i);
    fputc(')', stdout);
  } else {
    fputs(token(i), stdout);
  }
}

void print_quoted(int i)
{
  fputs("(quote ", stdout);
  print_expression(i);
  fputc(')', stdout);
}

int eval_list(int i)
{
  int retval;
  if (pair(first(i)))
    retval = i;
  else {
    char *p = token(first(i));
    if (strcmp(p, "quote") == 0)
      retval = first(rest(i));
    else if (strcmp(p, "first") == 0)
      retval = first(eval_expression(first(rest(i))));
    else if (strcmp(p, "rest") == 0)
      retval = rest(eval_expression(first(rest(i))));
    else if (strcmp(p, "pair") == 0)
      retval = make_pair(eval_expression(first(rest(i))),
                         eval_expression(first(rest(rest(i)))));
    else
      retval = i;
  };
  return retval;
}

int eval_expression(int i)
{
  int retval;
  if (pair(i)) {
    retval = eval_list(i);
    // retval = cells[cells[i].pair.rest].pair.first;
    //
    //   pair (not atom)
    //   eq (also compares with nil)
    // x car
    // x cdr
    // x cons
    //   null
    //
    //   cond
    //   define
    //   lambda
    //
    //   subst
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
  } else {
    retval = i;
  };
  return retval;
}

int main(void)
{
  int i;
  while (!feof(stdin)) {
    int expr = read_expression();
#ifndef NDEBUG
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
    fputc('\n', stderr);
#endif
    if (empty(expr)) break;
    print_quoted(eval_expression(expr)); fprintf(stdout, "\n");
    // print_quoted(expr); fprintf(stdout, "\n");
  };
  return 0;
}

