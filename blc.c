/* Bracket - Binary Lambda Calculus VM and DSL on top of it
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blc.h"

#define MAX_CELLS 256
#define MAX_REGISTERS 256

#define NIL -1

typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, INPUT } type_t;

typedef struct { int fun; int arg; } call_t;

typedef struct { int block; int env; } proc_t;

typedef struct {
  type_t type;
  union {
    int var;
    int lambda;
    call_t call;
    proc_t proc;
    FILE *input;
  };
  char mark;
} cell_t;

cell_t cells[MAX_CELLS];

int n_registers = 0;
int registers[MAX_REGISTERS];

int is_nil(int cell) { return cell == NIL; }

int type(int cell) { return cells[cell].type; }

int is_type(int cell, int t) { return is_nil(cell) ? 0 : type(cell) == t; }

int is_var(int cell) { return is_type(cell, VAR); }
int is_lambda(int cell) { return is_type(cell, LAMBDA); }
int is_call(int cell) { return is_type(cell, CALL); }
int is_proc(int cell) { return is_type(cell, PROC); }
int is_wrap(int cell) { return is_type(cell, WRAP); }
int is_input(int cell) { return is_type(cell, INPUT); }

int var(int cell) { return is_var(cell) ? cells[cell].var : NIL; }
int lambda(int cell) { return is_lambda(cell) ? cells[cell].lambda : NIL; }
int fun(int cell) { return is_call(cell) ? cells[cell].call.fun : NIL; }
int arg(int cell) { return is_call(cell) ? cells[cell].call.arg : NIL; }
int block(int cell) { return is_proc(cell) || is_wrap(cell) ? cells[cell].proc.block : NIL; }
int env(int cell) { return is_proc(cell) || is_wrap(cell) ? cells[cell].proc.env : NIL; }
FILE *input(int cell) { return is_input(cell) ? cells[cell].input : NULL; }

void clear_marks(void) {
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
  if (retval == MAX_CELLS) retval = NIL;
  return retval;
}

void mark(int expr)
{
  if (!cells[expr].mark) {
    cells[expr].mark = 1;
    switch (type(expr)) {
    case VAR:
      break;
    case LAMBDA:
      mark(lambda(expr));
      break;
    case CALL:
      mark(fun(expr));
      mark(arg(expr));
      break;
    case PROC:
    case WRAP:
      mark(block(expr));
      mark(env(expr));
      break;
    case INPUT:
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

int gc_push(int expr) { registers[n_registers++] = expr; return expr; }

void gc_pop(int n) { n_registers -= n; }

int cell(void)
{
#ifdef NDEBUG
  int retval = find_cell();
#else
  int retval = NIL;
#endif
  if (is_nil(retval)) {
    clear_marks();
    mark_registers();
    retval = find_cell();
  };
  if (retval != NIL) cells[retval].mark = 1;
  return retval;
}

int make_var(int var)
{
  int retval;
  if (var >= 0) {
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = VAR;
      cells[retval].var = var;
    };
  } else
    retval = NIL;
  return retval;
}

int make_lambda(int lambda)
{
  int retval;
  if (!is_nil(lambda)) {
    gc_push(lambda);
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = LAMBDA;
      cells[retval].lambda = lambda;
    };
    gc_pop(1);
  } else
    retval = NIL;
  return retval;
}

int make_call(int fun, int arg)
{
  int retval;
  if (!is_nil(fun) && !is_nil(arg)) {
    gc_push(fun);
    gc_push(arg);
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = CALL;
      cells[retval].call.fun = fun;
      cells[retval].call.arg = arg;
    };
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int make_proc(int block, int env)
{
  int retval;
  if (!is_nil(block) && !is_nil(env)) {
    gc_push(block);
    gc_push(env);
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = PROC;
      cells[retval].proc.block = block;
      cells[retval].proc.env = env;
    };
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int make_wrap(int block, int env)
{
  int retval;
  if (!is_nil(block) && !is_nil(env)) {
    gc_push(block);
    gc_push(env);
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = WRAP;
      cells[retval].proc.block = block;
      cells[retval].proc.env = env;
    };
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int make_input(FILE *input)
{
  int retval = cell();
  if (!is_nil(retval)) {
    cells[retval].type = INPUT;
    cells[retval].input = input;
  };
  return retval;
}

int make_false(void) { return make_lambda(make_lambda(make_var(0))); }

int is_false(int expr) { return var(lambda(lambda(expr))) == 0; }

int make_true(void) { return make_lambda(make_lambda(make_var(1))); }

int is_true(int expr) { return var(lambda(lambda(expr))) == 1; }

int read_bit(FILE *stream)
{
  int retval;
  int c = fgetc(stream);
  switch (c) {
  case '0':
    retval = make_false();
    break;
  case '1':
    retval = make_true();
    break;
  case EOF:
    retval = NIL;
    break;
  default:
    retval = read_bit(stream);
  };
  return retval;
}

int read_var(FILE *stream)
{
  int retval;
  int b = gc_push(read_bit(stream));
  if (is_false(b))
    retval = make_var(0);
  else if (is_true(b)) {
    retval = read_var(stream);
    if (!is_nil(retval)) cells[retval].var++;
  } else
    retval = NIL;
  gc_pop(1);
  return retval;
}

int read_lambda(FILE *stream)
{
  return make_lambda(read_expr(stream));
}

int read_call(FILE *stream)
{
  int fun = gc_push(read_expr(stream));
  int arg = gc_push(read_expr(stream));
  gc_pop(2);
  return make_call(fun, arg);
}

int read_expr(FILE *stream)
{
  int retval;
  int b1 = gc_push(read_bit(stream));
  if (is_false(b1)) {
    int b2 = gc_push(read_bit(stream));
    if (is_false(b2))
      retval = read_lambda(stream);
    else if (is_true(b2))
      retval = read_call(stream);
    else
      retval = NIL;
    gc_pop(1);
  } else if (is_true(b1))
    retval = read_var(stream);
  else
    retval = NIL;
  gc_pop(1);
  return retval;
}

int length(int list)
{
  int retval;
  if (!is_call(lambda(list)))
    retval = 0;
  else
    retval = 1 + length(arg(lambda(list)));
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

void print_call(int fun, int arg, FILE *stream)
{
  fputs("01", stream);
  print_expr(fun, stream);
  print_expr(arg, stream);
}

void print_proc(int block, int env, FILE *stream)
{
  fputs("#<proc:", stream);
  print_expr(block, stream);
  fprintf(stream, ";#env=%d>", length(env));
}

void print_wrap(int block, int env, FILE *stream)
{
  fputs("#<wrap:", stream);
  print_expr(block, stream);
  fprintf(stream, ";#env=%d>", length(env));
}

void print_expr(int expr, FILE *stream)
{
  if (!is_nil(expr)) {
    switch (type(expr)) {
    case VAR:
      print_var(var(expr), stream);
      break;
    case LAMBDA:
      print_lambda(lambda(expr), stream);
      break;
    case CALL:
      print_call(fun(expr), arg(expr), stream);
      break;
    case PROC:
      print_proc(block(expr), env(expr), stream);
      break;
    case WRAP:
      print_wrap(block(expr), env(expr), stream);
      break;
    case INPUT:
      fputs("#<input>", stream);
      break;
    }
  } else
    fputs("#<err>", stream);
}

int cons(int car, int cdr)
{
  int retval;
  if (!is_nil(car) && !is_nil(cdr)) {
    gc_push(car);
    gc_push(cdr);
    retval = make_lambda(make_call(make_call(make_var(0), car), cdr));
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int car(int list) { return arg(fun(lambda(list))); }

int cdr(int list) { return arg(lambda(list)); }

int lookup(int var, int env) { return var > 0 ? lookup(var - 1, cdr(env)) : car(env); }

int eval_expr(int expr, int local_env)
{
  int retval;
  int eval_fun;
  int wrap_arg;
  int call_env;
  int bit;
  gc_push(expr);
  gc_push(local_env);
  if (!is_nil(expr)) {
    switch (type(expr)) {
    case VAR:
      retval = lookup(var(expr), local_env);
      if (!is_nil(retval))
        retval = eval_expr(retval, local_env);
      else
        retval = make_var(var(expr) - length(local_env));
      break;
    case LAMBDA:
      retval = make_proc(lambda(expr), local_env);
      break;
    case CALL:
      eval_fun = gc_push(eval_expr(fun(expr), local_env));
      wrap_arg = gc_push(make_wrap(arg(expr), local_env));
      if (is_proc(eval_fun)) {
        call_env = gc_push(cons(wrap_arg, env(eval_fun)));
        retval = eval_expr(block(eval_fun), call_env);
        gc_pop(1);
      } else
        retval = eval_fun;
      gc_pop(2);
      break;
    case PROC:
      retval = expr;
      break;
    case WRAP:
      retval = eval_expr(block(expr), env(expr));
      break;
    case INPUT:
      bit = read_bit(input(expr));
      if (!is_nil(bit))
        retval = eval_expr(cons(bit, expr), local_env);
      else
        retval = eval_expr(make_false(), local_env);
      break;
    default:
      retval = NIL;
    }
  } else
    retval = NIL;
  gc_pop(2);
  return retval;
}
