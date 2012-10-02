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

#define MAX_PAIRS 128
#define TOKENSIZE 8
#define NIL -1

typedef enum { PAIR, SYMBOL } type_t;

typedef struct {
  int car;
  int cdr;
} pair_t;

typedef char symbol_t[TOKENSIZE + 3];

typedef struct {
  type_t type;
  union {
    pair_t pair;
    symbol_t symbol;
  };
} cell_t;

int n_cells = 0;
cell_t cells[MAX_PAIRS];

int read_token(void)
{
  int retval = n_cells++;
  int len = 0;
  char *p = cells[retval].symbol;
  cells[retval].type = SYMBOL;
  while (1) {
    char c = fgetc(stdin);
    if (c == EOF || c == '\n') break;
    *p++ = c;
    len++;
    if (len > TOKENSIZE) {
      *p = '\0';
      fprintf(stderr, "Error: Token %s... longer than %d characters\n",
              cells[retval].symbol, TOKENSIZE);
      exit(1);
    };
  };
  if (len <= 0)
    retval = NIL;
  else
    *p = '\0';
  return retval;
}

int pair(int i)
{
  return cells[i].type == PAIR;
}

char *symbol(int i)
{
  if (pair(i)) {
    fprintf(stderr, "Not a symbol\n");
    exit(1);
  };
  return cells[i].symbol;
}

int car(int i)
{
  if (!pair(i)) {
    fprintf(stderr, "Argument to 'car' must be a pair\n");
    exit(1);
  };
  return cells[i].pair.car;
}

int cdr(int i)
{
  if (!pair(i)) {
    fprintf(stderr, "Argument to 'cdr' must be a pair\n");
    exit(1);
  };
  return cells[i].pair.cdr;
}

void make_pair(int cell, int car, int cdr)
{
  cells[cell].type = PAIR;
  cells[cell].pair.car = car;
  cells[cell].pair.cdr = cdr;
}

int read_list(void)
{
  int retval;
  int cell = read_expression();
  if (cell != NIL) {
    retval = n_cells++;
    make_pair(retval, cell, read_list());
  } else {
    retval = NIL;
  };
  return retval;
}

int read_expression(void)
{
  int retval = read_token();
  if (retval != NIL) {
    char *str = symbol(retval);
    switch (str[0]) {
    case '(':
      retval = read_list();
      break;
    case ')':
      retval = NIL;
      break;
    };
  } else
    retval = NIL;
  return retval;
}

void print_expression(int i);

void print_list(int i)
{
  if (pair(i)) {
    print_expression(car(i));
    if (cdr(i) != NIL) {
      fputc(' ', stdout);
      print_list(cdr(i));
    }
  } else
    print_expression(i);
}

void print_expression(int i)
{
  if (i == NIL) {
    fputc('(', stdout);
    fputc(')', stdout);
  } else if (pair(i)) {
    fputc('(', stdout);
    print_list(i);
    fputc(')', stdout);
  } else {
    char *p = symbol(i);
    while (*p) fputc(*p++, stdout);
  }
}

int eval_list(int i)
{
  int retval;
  if (pair(car(i))) {
    char *p = symbol(car(i));
    if (strcmp(p, "car") == 0)
      retval = car(car(cdr(i)));
    else if (strcmp(p, "cdr") == 0)
      retval = cdr(car(cdr(i)));
    else
      retval = i;
  } else
    retval = i;
  return retval;
}

int eval_expression(int i)
{
  int retval;
  if (pair(i)) {
    retval = eval_list(i);
    // retval = cells[cells[i].pair.cdr].pair.car;
    //
    // pair (not atom)
    // eq (also compares with nil)
    // car
    // cdr
    // cons
    //
    // cond
    // define
    // lambda
    //
    // subst
    // equal
    // null
    // cadr
    // caddr
    // append
    // pair
    // assoc
    // sublis (hash)
    //
    // eval
    // quote
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
        fprintf(stderr, "car = %2d, cdr = %2d\n", car(i), cdr(i));
      else
        fprintf(stderr, "symbol = %s\n", symbol(i));
    };
    fputc('\n', stderr);
#endif
    if (expr == NIL) break;
    print_expression(eval_expression(expr)); fprintf(stdout, "\n");
    // print_expression(expr); fprintf(stdout, "\n");
  };
  return 0;
}

