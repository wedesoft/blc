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
#include "blc.h"
#define MAX_CELLS 1024

typedef enum { VAR, LAMBDA, PAIR } type_t;

typedef struct {
  int fun;
  int arg;
} pair_t;

typedef struct {
  type_t type;
  union {
    int var;
    int lambda;
    pair_t pair;
  };
} cell_t;

int n_cells = 0;
cell_t cells[MAX_CELLS];

int cell(void)
{
  int retval;
  if (n_cells < MAX_CELLS) {
    retval = n_cells++;
  } else
    retval = -1;
  return retval;
}

int read_bit(FILE *stream)
{
  int retval;
  int c = fgetc(stream);
  switch (c) {
  case '0':
    retval = 0;
    break;
  case '1':
    retval = 1;
    break;
  case EOF:
    retval = -1;
    break;
  default:
    retval = read_bit(stream);
  };
  return retval;
}

int read_var(FILE *stream)
{
  int retval;
  int b = read_bit(stream);
  if (b == 0) {
    retval = cell();
    if (retval >= 0) {
      cells[retval].type = VAR;
      cells[retval].var = 0;
    };
  } else if (b == 1) {
    retval = read_var(stream);
    if (retval >= 0) cells[retval].var++;
  } else
    retval = -1;
  return retval;
}

void print_var(int var, FILE *stream)
{
  fputc('1', stream);
  if (var > 0)
    print_var(var - 1, stream);
  else
    fputc('0', stream);
}

void print_lambda(int lambda, FILE *stream)
{
  fputs("00", stream);
  print_expr(lambda, stream);
}

void print_pair(int fun, int arg, FILE *stream)
{
  fputs("01", stream);
  print_expr(fun, stream);
  print_expr(arg, stream);
}

int read_lambda(FILE *stream)
{
  int retval;
  int lambda = read_expr(stream);
  if (lambda >= 0) {
    retval = cell();
    if (retval >= 0) {
      cells[retval].type = LAMBDA;
      cells[retval].lambda = lambda;
    }
  } else
    retval = -1;
  return retval;
}

int read_pair(FILE *stream)
{
  int retval;
  int fun = read_expr(stream);
  int arg = read_expr(stream);
  if (fun >= 0 && arg >= 0) {
    retval = cell();
    if (retval >= 0) {
      cells[retval].type = PAIR;
      cells[retval].pair.fun = fun;
      cells[retval].pair.arg = arg;
    }
  } else
    retval = -1;
  return retval;
}

int read_expr(FILE *stream)
{
  int retval;
  int b1 = read_bit(stream);
  if (b1 == 0) {
    int b2 = read_bit(stream);
    if (b2 == 0)
      retval = read_lambda(stream);
    else if (b2 == 1)
      retval = read_pair(stream);
    else
      retval = -1;
  } else if (b1 == 1)
    retval = read_var(stream);
  else
    retval = -1;
  return retval;
}

void print_expr(int expr, FILE *stream)
{
  if (expr >= 0) {
    switch (cells[expr].type) {
    case VAR:
      print_var(cells[expr].var, stream);
      break;
    case LAMBDA:
      print_lambda(cells[expr].lambda, stream);
      break;
    case PAIR:
      print_pair(cells[expr].pair.fun, cells[expr].pair.arg, stream);
      break;
    }
  } else
    fputs("#<err>", stream);
}
