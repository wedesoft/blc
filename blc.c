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

#define NIL -1

typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, STDIN } type_t;

typedef struct {
  int fun;
  int arg;
} call_t;

typedef struct {
  int block;
  int env;
} proc_t;

typedef struct {
  int block;
  int env;
} wrap_t;

typedef struct {
  type_t type;
  union {
    int var;
    int lambda;
    call_t call;
    proc_t proc;
    wrap_t wrap;
    // void stdin;
  };
  char mark;
} cell_t;

cell_t cells[MAX_CELLS];

int n_registers = 0;
int registers[MAX_REGISTERS];

int is_nil(int cell)
{
  return cell == NIL;
}

int type(int cell)
{
  return cells[cell].type;
}

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
    retval = NIL;
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
      mark(cells[expr].lambda);
      break;
    case CALL:
      mark(cells[expr].call.fun);
      mark(cells[expr].call.arg);
      break;
    case PROC:
      mark(cells[expr].proc.block);
      mark(cells[expr].proc.env);
      break;
    case WRAP:
      mark(cells[expr].wrap.block);
      mark(cells[expr].wrap.env);
      break;
    case STDIN:
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
  if (is_nil(retval)) {
    clear_marks();
    mark_registers();
    retval = find_cell();
  };
  if (retval != NIL)
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
    retval = NIL;
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
    if (!is_nil(retval)) {
      cells[retval].type = VAR;
      cells[retval].var = var;
    };
  } else
    retval = NIL;
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
    if (!is_nil(retval)) cells[retval].var++;
  } else
    retval = NIL;
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

int is_var(int cell)
{
  return is_nil(cell) ? 0 : type(cell) == VAR;
}

int make_lambda(int lambda)
{
  int retval;
  gc_push(lambda);
  if (!is_nil(lambda)) {
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = LAMBDA;
      cells[retval].lambda = lambda;
    };
  } else
    retval = NIL;
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

int is_lambda(int cell)
{
  return is_nil(cell) ? 0 : type(cell) == LAMBDA;
}

int make_call(int fun, int arg)
{
  int retval;
  gc_push(fun);
  gc_push(arg);
  if (!is_nil(fun) && !is_nil(arg)) {
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = CALL;
      cells[retval].call.fun = fun;
      cells[retval].call.arg = arg;
    };
  } else
    retval = NIL;
  gc_pop(2);
  return retval;
}

int read_call(FILE *stream)
{
  int fun = gc_push(read_expr(stream));
  int arg = gc_push(read_expr(stream));
  gc_pop(2);
  return make_call(fun, arg);
}

void print_call(int fun, int arg, FILE *stream)
{
  fputs("01", stream);
  print_expr(fun, stream);
  print_expr(arg, stream);
}

int is_call(int cell)
{
  return is_nil(cell) ? 0 : type(cell) == CALL;
}

int make_proc(int block, int env)
{
  int retval;
  gc_push(block);
  gc_push(env);
  if (!is_nil(block) && !is_nil(env)) {
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = PROC;
      cells[retval].proc.block = block;
      cells[retval].proc.env = env;
    };
  } else
    retval = NIL;
  gc_pop(2);
  return retval;
}

int length(int list);

void print_proc(int block, int env, FILE *stream)
{
  fputs("#<proc:", stream);
  print_expr(block, stream);
  fprintf(stream, ";#env=%d>", length(env));
}

int is_proc(int cell)
{
  return is_nil(cell) ? 0 : type(cell) == PROC;
}

int make_wrap(int block, int env)
{
  int retval;
  gc_push(block);
  gc_push(env);
  if (!is_nil(block) && !is_nil(env)) {
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = WRAP;
      cells[retval].wrap.block = block;
      cells[retval].wrap.env = env;
    };
  } else
    retval = NIL;
  gc_pop(2);
  return retval;
}

void print_wrap(int block, int env, FILE *stream)
{
  fputs("#<wrap:", stream);
  print_expr(block, stream);
  fprintf(stream, ";#env=%d>", length(env));
}

int is_wrap(int cell)
{
  return is_nil(cell) ? 0 : type(cell) == WRAP;
}

int make_stdin(void)
{
  int retval = cell();
  if (!is_nil(retval))
    cells[retval].type = STDIN;
  return retval;
}

int is_stdin(int cell)
{
  return is_nil(cell) ? 0 : type(cell) == STDIN;
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
      retval = read_call(stream);
    else
      retval = NIL;
  } else if (b1 == 1)
    retval = read_var(stream);
  else
    retval = NIL;
  return retval;
}

void print_expr(int expr, FILE *stream)
{
  if (!is_nil(expr)) {
    switch (type(expr)) {
    case VAR:
      print_var(cells[expr].var, stream);
      break;
    case LAMBDA:
      print_lambda(cells[expr].lambda, stream);
      break;
    case CALL:
      print_call(cells[expr].call.fun, cells[expr].call.arg, stream);
      break;
    case PROC:
      print_proc(cells[expr].proc.block, cells[expr].proc.env, stream);
      break;
    case WRAP:
      print_wrap(cells[expr].wrap.block, cells[expr].wrap.env, stream);
      break;
    case STDIN:
      fputs("#<stdin>", stream);
      break;
    default:
      fputs("#<err>", stream);
    }
  } else
    fputs("#<err>", stream);
}

int cons(int car, int cdr)
{
  int retval;
  gc_push(car);
  gc_push(cdr);
  retval = make_lambda(make_call(make_call(make_var(0), car), cdr));
  gc_pop(2);
  return retval;
}

int car(int list)
{
  int retval;
  if (!is_lambda(list))
    retval = NIL;
  else if (!is_call(cells[list].lambda))
    retval = NIL;
  else if (!is_call(cells[cells[list].lambda].call.fun))
    retval = NIL;
  else
    retval = cells[cells[cells[list].lambda].call.fun].call.arg;
  return retval;
}

int cdr(int list)
{
  int retval;
  if (!is_lambda(list))
    retval = NIL;
  else if (!is_call(cells[list].lambda))
    retval = NIL;
  else
    retval = cells[cells[list].lambda].call.arg;
  return retval;
}

int lookup(int var, int env)
{
  int retval;
  if (var > 0)
    retval = lookup(var - 1, cdr(env));
  else
    retval = car(env);
  return retval;
}

int length(int list)
{
  int retval;
  if (!is_call(cells[list].lambda))
    retval = 0;
  else
    retval = 1 + length(cells[cells[list].lambda].call.arg);
  return retval;
}

int eval_expr(int expr, int env)
{
#ifndef NDEBUG
  // print_expr(expr, stderr); fputs("\n", stderr);
#endif
  int retval;
  int fun;
  int arg;
  int local_env;
  gc_push(expr);
  gc_push(env);
  if (!is_nil(expr)) {
    switch (type(expr)) {
    case VAR:
      retval = lookup(cells[expr].var, env);
      if (is_nil(retval))
        retval = make_var(cells[expr].var - length(env));
      else
        retval = eval_expr(retval, env);
      break;
    case LAMBDA:
      retval = make_proc(cells[expr].lambda, env);
      break;
    case CALL:
      fun = gc_push(eval_expr(cells[expr].call.fun, env));
      arg = gc_push(make_wrap(cells[expr].call.arg, env));
      if (is_proc(fun)) {
        local_env = gc_push(cons(arg, cells[fun].proc.env));
        retval = eval_expr(cells[fun].proc.block, local_env);
      } else
        retval = eval_expr(fun, env);
      gc_pop(3);
      break;
    case PROC:
      retval = expr;
      break;
    case WRAP:
      retval = eval_expr(cells[expr].wrap.block, cells[expr].wrap.env);
      break;
    case STDIN:
      retval = expr;
      // retval = make_lambda(make_call(make_call(make_var(0), read_bit(stdin) ? make_true() : make_false()), expr));
      // retval = make_proc(make_call(make_call(make_var(0), make_var(1)), make_var(2)), env);
      break;
    default:
      retval = NIL;
    }
  } else
    retval = NIL;
  gc_pop(2);
#ifndef NDEBUG
  //print_expr(expr, stderr);
  //fputs(" -> ", stderr);
  //print_expr(retval, stderr);
  //puts("\n", stderr);
#endif
  return retval;
}
