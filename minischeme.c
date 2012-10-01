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

typedef enum {
  PAIR = 0,
  SYMBOL
} type_t;

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

char *read_token(char *str)
{
  int len = 0;
  char *retval;
  char *p = str;
  while (1) {
    char c = fgetc(stdin);
    if (c == EOF || c == '\n') break;
    *p++ = c;
    len++;
    if (len > TOKENSIZE) {
      *p = '\0';
      fprintf(stderr, "Error: Token %s... longer than %d characters\n",
              str, TOKENSIZE);
      exit(1);
    };
  };
  if (len <= 0)
    retval = NULL;
  else {
    *p = '\0';
    retval = str;
  };
  return retval;
}

int n_cells = 0;
cell_t cells[MAX_PAIRS];

int read_list(void)
{
  int retval;
  int cell = read_expression();
  if (cell != NIL) {
    retval = n_cells++;
    cells[retval].type = PAIR;
    cells[retval].pair.car = cell;
    cells[retval].pair.cdr = read_list();
  } else {
    retval = NIL;
  };
  return retval;
}

void make_pair(int cell, int car, int cdr)
{
  cells[cell].type = PAIR;
  cells[cell].pair.car = car;
  cells[cell].pair.cdr = cdr;
}

int read_expression(void)
{
  int retval = n_cells;
  char *str = read_token(cells[retval].symbol);
  if (str) {
    switch (str[0]) {
    case '(':
      // n_cells++;
      retval = read_list();
      // make_pair(retval, read_list(), NIL);
      break;
    case ')':
      retval = NIL;
      break;
    default:
      n_cells++;
      cells[retval].type = SYMBOL;
    };
  } else
    retval = NIL;
  return retval;
}

void print_expression(int i);

void print_list(int i)
{
  if (cells[i].type == PAIR) {
    print_expression(cells[i].pair.car);
    if (cells[i].pair.cdr != NIL) {
      fputc(' ', stdout);
      print_list(cells[i].pair.cdr);
    }
  };
}

void print_expression(int i)
{
  if (i == NIL) {
    fputc('(', stdout);
    fputc(')', stdout);
  } else if (cells[i].type == PAIR) {
    fputc('(', stdout);
    print_list(i);
    fputc(')', stdout);
  } else {
    char *symbol = cells[i].symbol;
    while (*symbol) fputc(*symbol++, stdout);
  }
}

int eval(int i)
{
  int retval;
  if (cells[i].type == PAIR) {
    retval = i;
    // retval = cells[cells[i].pair.cdr].pair.car;
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
      if (cells[i].type == PAIR)
        fprintf(stderr, "car = %2d, cdr = %2d\n", cells[i].pair.car, cells[i].pair.cdr);
      else
        fprintf(stderr, "symbol = %s\n", cells[i].symbol);
    };
    fputc('\n', stderr);
#endif
    if (expr == NIL) break;
    print_expression(eval(expr)); fprintf(stdout, "\n");
    // print_expression(expr); fprintf(stdout, "\n");
  };
  return 0;
}

