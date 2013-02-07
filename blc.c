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
#define MAX_CELLS 256
#define MAX_REGISTERS 256

typedef enum { VAR, LAMBDA, PAIR, PROC } type_t;

typedef struct {
  int fun;
  int arg;
} pair_t;

typedef struct {
  int fun;
  int env;
} proc_t;

typedef struct {
  type_t type;
  union {
    int var;
    int lambda;
    pair_t pair;
    proc_t proc;
  };
  char mark;
} cell_t;

cell_t cells[MAX_CELLS];

int n_registers = 0;
int registers[MAX_REGISTERS];

void clear_marks(void)
{
  int i;
  for (i=0; i<MAX_CELLS; i++)
    cells[i].mark = 0;
}

int find_cell(void)
{
  int retval = 0;
  while (cells[retval].mark) {
    retval++;
    if (retval == MAX_CELLS) break;
  };
  if (retval == MAX_CELLS)
    retval = -1;
  return retval;
}

void mark(int expr)
{
  if (!cells[expr].mark) {
    cells[expr].mark = 1;
    switch (cells[expr].type) {
    case VAR:
      break;
    case LAMBDA:
      mark(cells[expr].lambda);
      break;
    case PAIR:
      mark(cells[expr].pair.fun);
      mark(cells[expr].pair.arg);
      break;
    case PROC:
      mark(cells[expr].proc.fun);
      mark(cells[expr].proc.env);
      break;
    }
  };
}

void mark_registers(void)
{
  int i;
  for (i=0; i<n_registers; i++)
    mark(registers[i]);
}

int gc_push(int expr)
{
  registers[n_registers++] = expr;
  return expr;
}

void gc_pop(int n)
{
  n_registers -= n;
}

int cell(void)
{
  int retval = find_cell();
  if (retval == -1) {
    clear_marks();
    mark_registers();
    retval = find_cell();
  };
  if (retval != -1)
    cells[retval].mark = 1;
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
  gc_push(lambda);
  if (lambda >= 0) {
    retval = cell();
    if (retval >= 0) {
      cells[retval].type = LAMBDA;
      cells[retval].lambda = lambda;
    };
  } else
    retval = -1;
  gc_pop(1);
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
  gc_push(fun);
  gc_push(arg);
  if (fun >= 0 && arg >= 0) {
    retval = cell();
    if (retval >= 0) {
      cells[retval].type = PAIR;
      cells[retval].pair.fun = fun;
      cells[retval].pair.arg = arg;
    };
  } else
    retval = -1;
  gc_pop(2);
  return retval;
}

int read_pair(FILE *stream)
{
  int fun = gc_push(read_expr(stream));
  int arg = gc_push(read_expr(stream));
  gc_pop(2);
  return make_pair(fun, arg);
}

void print_pair(int fun, int arg, FILE *stream)
{
  fputs("01", stream);
  print_expr(fun, stream);
  print_expr(arg, stream);
}

int make_proc(int fun, int env)
{
  int retval;
  gc_push(fun);
  gc_push(env);
  if (fun >= 0 && env >= 0) {
    retval = cell();
    cells[retval].type = PROC;
    cells[retval].proc.fun = fun;
    cells[retval].proc.env = env;
  } else
    retval = -1;
  gc_pop(2);
  return retval;
}

void print_proc(int fun, int env, FILE *stream)
{
  fputs("#<proc>", stream);
}

int make_false(void)
{
  return make_lambda(make_lambda(make_var(0)));
}

int make_true(void)
{
  return make_lambda(make_lambda(make_var(1)));
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
    case PROC:
      print_proc(cells[expr].proc.fun, cells[expr].proc.env, stream);
      break;
    default:
      fputs("#<err>", stream);
    }
  } else
    fputs("#<err>", stream);
}

int push_env(int env, int var)
{
  return make_lambda(make_pair(make_pair(make_var(0), var), env));
}

int lookup(int var, int env)
{
  int retval;
  if (cells[cells[env].lambda].type != PAIR)
    retval = -1;
  else if (var > 0)
    retval = lookup(var - 1, cells[cells[env].lambda].pair.arg);
  else
    retval = cells[cells[cells[env].lambda].pair.fun].pair.arg;
  return retval;
}

int depth(int env)
{
  int retval;
  if (cells[cells[env].lambda].type != PAIR)
    retval = 0;
  else
    retval = 1 + depth(cells[cells[env].lambda].pair.arg);
  return retval;
}

int eval_expr(int expr, int env)
{
  int retval;
  int fun;
  int arg;
  int local_env;
  gc_push(expr);
  gc_push(env);
  if (expr >= 0) {
    switch (cells[expr].type) {
    case VAR:
      retval = lookup(cells[expr].var, env);
      if (retval == -1) retval = make_var(cells[expr].var - depth(env));
      break;
    case LAMBDA:
      retval = make_proc(cells[expr].lambda, env);
      break;
    case PAIR:
      fun = gc_push(eval_expr(cells[expr].pair.fun, env));
      arg = gc_push(cells[expr].pair.arg);
      local_env = gc_push(push_env(cells[fun].proc.env, arg));
      if (cells[fun].type == PROC) {
        retval = eval_expr(cells[fun].proc.fun, local_env);
      } else
        retval = -1;
      gc_pop(3);
      break;
    case PROC:
      retval = expr;
      break;
    default:
      retval = -1;
    }
  } else
    retval = -1;
  gc_pop(2);
  return retval;
}

