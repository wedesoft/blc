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

int make_var(int var)
{
  int retval;
  if (var >= 0) {
    retval = cell();
    if (retval >= 0) {
      cells[retval].type = VAR;
      cells[retval].var = var;
    };
  } else
    retval = -1;
  return retval;
}

int read_var(FILE *stream)
{
  int retval;
  int b = read_bit(stream);
  if (b == 0)
    retval = make_var(0);
  else if (b == 1) {
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

int make_lambda(int lambda)
{
  int retval;
  if (lambda >= 0) {
    retval = cell();
    if (retval >= 0) {
      cells[retval].type = LAMBDA;
      cells[retval].lambda = lambda;
    };
  } else
    retval = -1;
  return retval;
}

int read_lambda(FILE *stream)
{
  return make_lambda(read_expr(stream));
}

void print_lambda(int lambda, FILE *stream)
{
  fputs("00", stream);
  print_expr(lambda, stream);
}

int make_pair(int fun, int arg)
{
  int retval;
  if (fun >= 0 && arg >= 0) {
    retval = cell();
    if (retval >= 0) {
      cells[retval].type = PAIR;
      cells[retval].pair.fun = fun;
      cells[retval].pair.arg = arg;
    };
  } else
    retval = -1;
  return retval;
}

int read_pair(FILE *stream)
{
  int fun = read_expr(stream);
  int arg = read_expr(stream);
  return make_pair(fun, arg);
}

void print_pair(int fun, int arg, FILE *stream)
{
  fputs("01", stream);
  print_expr(fun, stream);
  print_expr(arg, stream);
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
    default:
      fputs("#<err>", stream);
    }
  } else
    fputs("#<err>", stream);
}

int offset(int expr)
{
  int retval;
  if (expr >= 0) {
    switch (cells[expr].type) {
    case VAR:
      retval = 0;
      break;
    case LAMBDA:
      retval = 1 + offset(cells[expr].lambda);
      break;
    case PAIR:
      retval = offset(cells[expr].pair.fun) + offset(cells[expr].pair.arg);
      break;
    default:
      retval = 0;
    }
  } else
    retval = 0;
  return retval;
}

int lift(int expr, int amount)
{
  int retval;
  if (expr >= 0) {
    switch (cells[expr].type) {
    case VAR:
      retval = make_var(cells[expr].var + amount);
      break;
    case LAMBDA:
      retval = make_lambda(lift(cells[expr].lambda, amount));
      break;
    case PAIR:
      retval = make_pair(lift(cells[expr].pair.fun, amount),
                         lift(cells[expr].pair.arg, amount));
      break;
    default:
      retval = 0;
    }
  } else
    retval = -1;
  return retval;
}

int subst(int expr, int var, int replacement)
{
  int retval;
  if (expr >= 0)
    retval = expr;
  else
    retval = -1;
  return retval;
}

int eval_expr(int expr)
{
  int retval;
  int fun;
  int arg;
  if (expr >= 0) {
    switch (cells[expr].type) {
    case VAR:
    case LAMBDA:
      retval = expr;
      break;
    case PAIR:
      fun = eval_expr(cells[expr].pair.fun);
      arg = cells[expr].pair.arg;
      retval = subst(fun, 0, arg);
      break;
    default:
      retval = -1;
    }
  } else
    retval = -1;
  return retval;
}

