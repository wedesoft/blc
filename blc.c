/* BLC - Binary Lambda Calculus VM and DSL on top of it
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

#define MAX_CELLS 1024
#define MAX_REGISTERS 1024

#define NIL -1

typedef enum { VARIABLE, LAMBDA, CALL, PROC, WRAP, INPUT } type_t;

typedef struct { int function; int argument; } call_t;

typedef struct { int function; int environment; } proc_t;

typedef struct { int unwrap; int environment; } wrap_t;

typedef struct { FILE *file; int used; } input_t;

typedef struct {
  type_t type;
  union {
    int variable;
    int lambda;
    call_t call;
    proc_t proc;
    wrap_t wrap;
    input_t input;
  };
  char mark;
} cell_t;

cell_t cells[MAX_CELLS];

int n_registers = 0;
int registers[MAX_REGISTERS];

int is_nil(int cell) { return cell == NIL; }

int type(int cell) { return cells[cell].type; }

int is_type(int cell, int t) { return is_nil(cell) ? 0 : type(cell) == t; }

int is_variable(int cell) { return is_type(cell, VARIABLE); }
int is_lambda(int cell) { return is_type(cell, LAMBDA); }
int is_call(int cell) { return is_type(cell, CALL); }
int is_proc(int cell) { return is_type(cell, PROC); }
int is_wrap(int cell) { return is_type(cell, WRAP); }
int is_input(int cell) { return is_type(cell, INPUT); }

int variable(int cell) { return is_variable(cell) ? cells[cell].variable : NIL; }
int function(int cell) { return is_call(cell) ? cells[cell].call.function : is_proc(cell) ? cells[cell].proc.function : is_lambda(cell) ? cells[cell].lambda : NIL; }
int argument(int cell) { return is_call(cell) ? cells[cell].call.argument : NIL; }
int unwrap(int cell) { return is_wrap(cell) ? cells[cell].wrap.unwrap : NIL; }
int environment(int cell) { return is_proc(cell) ? cells[cell].proc.environment : is_wrap(cell) ? cells[cell].wrap.environment : NIL; }
FILE *file(int cell) { return is_input(cell) ? cells[cell].input.file : NULL; }

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

void mark(int expression)
{
  if (!cells[expression].mark) {
    cells[expression].mark = 1;
    switch (type(expression)) {
    case VARIABLE:
      break;
    case LAMBDA:
      mark(function(expression));
      break;
    case CALL:
      mark(function(expression));
      mark(argument(expression));
      break;
    case PROC:
      mark(function(expression));
      mark(environment(expression));
      break;
    case WRAP:
      mark(unwrap(expression));
      mark(environment(expression));
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

int gc_push(int expression) { registers[n_registers++] = expression; return expression; }

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
  if (!is_nil(retval)) cells[retval].mark = 1;
  return retval;
}

int make_variable(int variable)
{
  int retval;
  if (variable >= 0) {
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = VARIABLE;
      cells[retval].variable = variable;
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

int make_call(int function, int argument)
{
  int retval;
  if (!is_nil(function) && !is_nil(argument)) {
    gc_push(function);
    gc_push(argument);
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = CALL;
      cells[retval].call.function = function;
      cells[retval].call.argument = argument;
    };
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int make_proc(int function, int environment)
{
  int retval;
  if (!is_nil(function) && !is_nil(environment)) {
    gc_push(function);
    gc_push(environment);
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = PROC;
      cells[retval].proc.function = function;
      cells[retval].proc.environment = environment;
    };
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int make_wrap(int unwrap, int environment)
{
  int retval;
  if (!is_nil(unwrap) && !is_nil(environment)) {
    gc_push(unwrap);
    gc_push(environment);
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = WRAP;
      cells[retval].wrap.unwrap = unwrap;
      cells[retval].wrap.environment = environment;
    };
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int make_input(FILE *file)
{
  int retval = cell();
  if (!is_nil(retval)) {
    cells[retval].type = INPUT;
    cells[retval].input.file = file;
    cells[retval].input.used = 0;
  };
  return retval;
}

int make_false(void) { return make_lambda(make_lambda(make_variable(0))); }

int is_false(int expression) { return variable(function(function(expression))) == 0; }

int make_true(void) { return make_lambda(make_lambda(make_variable(1))); }

int is_true(int expression) { return variable(function(function(expression))) == 1; }

int read_bit(int input)
{
  int retval;
  if (cells[input].input.used)
    retval = NIL;
  else {
    int c = fgetc(file(gc_push(input)));
    switch (c) {
    case '0':
      retval = make_pair(gc_push(make_input(file(input))), gc_push(make_false()));
      gc_pop(2);
      break;
    case '1':
      retval = make_pair(gc_push(make_input(file(input))), gc_push(make_true()));
      gc_pop(2);
      break;
    case EOF:
      retval = NIL;
      break;
    default:
      retval = read_bit(input);
    };
    cells[input].input.used = 1;
    gc_pop(1);
  };
  return retval;
}

int make_pair(int first, int second)
{
  int retval;
  if (!is_nil(first) && !is_nil(second)) {
    gc_push(first);
    gc_push(second);
    retval = make_lambda(make_call(make_call(make_variable(0), first), second));
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int is_pair(int expression) { return variable(function(function(function(expression)))) == 0; }

int first(int list)
{
  return argument(function(function(list)));
}

int second(int list) {
  int retval;
  retval = argument(function(gc_push(list)));
  gc_pop(1);
  return retval;
}

int read_variable(int input)
{
  int retval;
  int b = gc_push(read_bit(gc_push(input)));
  if (is_false(second(b))) {
    retval = make_pair(first(b), gc_push(make_variable(0)));
    gc_pop(1);
  } else if (is_true(second(b))) {
    retval = read_variable(first(b));
    if (!is_nil(retval)) cells[second(retval)].variable++;
  } else
    retval = NIL;
  gc_pop(2);
  return retval;
}

int read_lambda(int input)
{
  int retval;
  int term = gc_push(read_expression(input));
  if (!is_nil(term)) {
    retval = make_pair(first(term), gc_push(make_lambda(second(term))));
    gc_pop(1);
  } else
    retval = NIL;
  gc_pop(1);
  return retval;
}

int read_call(int input)
{
  int retval;
  int function = gc_push(read_expression(input));
  if (!is_nil(function)) {
    int argument = gc_push(read_expression(first(function)));
    if (!is_nil(argument)) {
      retval = make_pair(first(argument), gc_push(make_call(second(function), second(argument))));
      gc_pop(1);
    } else
      retval = NIL;
    gc_pop(1);
  } else
    retval = NIL;
  gc_pop(1);
  return retval;
}

int read_expression(int input)
{
  int retval;
  int b1 = gc_push(read_bit(gc_push(input)));
  if (is_false(second(b1))) {
    int b2 = gc_push(read_bit(first(b1)));
    if (is_false(second(b2)))
      retval = read_lambda(first(b2));
    else if (is_true(second(b2)))
      retval = read_call(first(b2));
    else
      retval = NIL;
    gc_pop(1);
  } else if (is_true(second(b1)))
    retval = read_variable(first(b1));
  else
    retval = NIL;
  gc_pop(2);
  return retval;
}

int length(int list)
{
  int retval;
  if (!is_pair(list))
    retval = 0;
  else
    retval = 1 + length(second(list));
  return retval;
}

void print_variable(int variable, FILE *file)
{
  fputc('1', file);
  if (variable > 0)
    print_variable(variable - 1, file);
  else
    fputc('0', file);
}

void print_lambda(int lambda, FILE *file)
{
  fputs("00", file);
  print_expression(lambda, file);
}

void print_call(int function, int argument, FILE *file)
{
  fputs("01", file);
  print_expression(function, file);
  print_expression(argument, file);
}

void print_proc(int function, int environment, FILE *file)
{
  fputs("#<proc:", file);
  print_expression(function, file);
  fprintf(file, ";#env=%d>", length(environment));
}

void print_wrap(int unwrap, int environment, FILE *file)
{
  fputs("#<wrap:", file);
  print_expression(unwrap, file);
  fprintf(file, ";#env=%d>", length(environment));
}

void print_expression(int expression, FILE *file)
{
  if (!is_nil(expression)) {
    switch (type(expression)) {
    case VARIABLE:
      print_variable(variable(expression), file);
      break;
    case LAMBDA:
      print_lambda(function(expression), file);
      break;
    case CALL:
      print_call(function(expression), argument(expression), file);
      break;
    case PROC:
      print_proc(function(expression), environment(expression), file);
      break;
    case WRAP:
      print_wrap(unwrap(expression), environment(expression), file);
      break;
    case INPUT:
      fputs("#<input>", file);
      break;
    }
  } else
    fputs("#<err>", file);
}

int lookup(int variable, int environment) { return variable > 0 ? lookup(variable - 1, second(environment)) : first(environment); }

int eval_expression(int expression, int local_environment)
{
  int retval;
  int eval_fun;
  int wrap_argument;
  int call_environment;
  int bit;
  gc_push(expression);
  gc_push(local_environment);
  if (!is_nil(expression)) {
    switch (type(expression)) {
    case VARIABLE:
      retval = lookup(variable(expression), local_environment);
      if (!is_nil(retval))
        retval = eval_expression(retval, local_environment);
      else
        retval = make_variable(variable(expression) - length(local_environment));
      break;
    case LAMBDA:
      retval = make_proc(function(expression), local_environment);
      break;
    case CALL:
      eval_fun = gc_push(eval_expression(function(expression), local_environment));
      wrap_argument = gc_push(make_wrap(argument(expression), local_environment));
      if (is_proc(eval_fun)) {
        call_environment = gc_push(make_pair(wrap_argument, environment(eval_fun)));
        retval = eval_expression(function(eval_fun), call_environment);
        gc_pop(1);
      } else
        retval = eval_fun;
      gc_pop(2);
      break;
    case PROC:
      retval = expression;
      break;
    case WRAP:
      retval = eval_expression(unwrap(expression), environment(expression));
      break;
    case INPUT:
      bit = gc_push(read_bit(expression));
      if (!is_nil(bit))
        retval = eval_expression(gc_push(make_pair(second(bit), first(bit))), local_environment);
      else
        retval = eval_expression(gc_push(make_false()), local_environment);
      gc_pop(2);
      break;
    default:
      retval = NIL;
    }
  } else
    retval = NIL;
  gc_pop(2);
  return retval;
}
