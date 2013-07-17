/* BLC - Binary Lambda Calculus VM
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

#define MAX_CELLS 1000000
#define MAX_REGISTERS 4000000

typedef enum { VARIABLE, LAMBDA, CALL, PROC, WRAP, INPUT, OUTPUT } type_t;

typedef struct { int function; int argument; } call_t;

typedef struct { int term; int body; } definition_t;

typedef struct { int function; int environment; } proc_t;

typedef struct { int unwrap; int environment; } wrap_t;

typedef struct { FILE *file; int used; } input_t;

typedef struct { FILE *file; int used; } output_t;

typedef struct {
  type_t type;
  union {
    int variable;
    int lambda;
    call_t call;
    definition_t definition;
    proc_t proc;
    wrap_t wrap;
    input_t input;
    output_t output;
  };
  char mark;
} cell_t;

cell_t cells[MAX_CELLS];

int n_registers = 0;
int registers[MAX_REGISTERS];

int type(int cell) { return cells[cell].type; }

int is_type(int cell, int t) { return type(cell) == t; }

int is_variable(int cell) { return is_type(cell, VARIABLE); }
int is_lambda(int cell) { return is_type(cell, LAMBDA); }
int is_call(int cell) { return is_type(cell, CALL); }
int is_proc(int cell) { return is_type(cell, PROC); }
int is_wrap(int cell) { return is_type(cell, WRAP); }
int is_input(int cell) { return is_type(cell, INPUT); }
int is_output(int cell) { return is_type(cell, OUTPUT); }

int variable(int cell) { return cells[cell].variable; }
int has_function(int cell) { return is_call(cell) || is_proc(cell) || is_lambda(cell); }
int function(int cell) { return is_call(cell) ? cells[cell].call.function : is_proc(cell) ? cells[cell].proc.function : cells[cell].lambda; }
int argument(int cell) { return cells[cell].call.argument; }
int term(int cell) { return cells[cell].definition.term; }
int body(int cell) { return cells[cell].definition.body; }
int unwrap(int cell) { return cells[cell].wrap.unwrap; }
int environment(int cell) { return is_proc(cell) ? cells[cell].proc.environment : cells[cell].wrap.environment; }
FILE *file(int cell) { return is_input(cell) ? cells[cell].input.file : is_output(cell) ? cells[cell].output.file : NULL; }
int used(int cell) { return is_input(cell) ? cells[cell].input.used : is_output(cell) ? cells[cell].output.used : cell; }

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
      case OUTPUT:
        mark(used(expression));
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
  int retval = MAX_CELLS;
#endif
  if (retval == MAX_CELLS) {
    clear_marks();
    mark_registers();
    retval = find_cell();
    if (retval == MAX_CELLS) {
      fprintf(stderr, "Out of memory!\n");
      exit(1);
    };
  };
  cells[retval].mark = 1;
  return retval;
}

int make_variable(int variable)
{
  int retval = cell();
  cells[retval].type = VARIABLE;
  cells[retval].variable = variable;
  return retval;
}

int make_lambda(int lambda)
{
  gc_push(lambda);
  int retval = cell();
  cells[retval].type = LAMBDA;
  cells[retval].lambda = lambda;
  gc_pop(1);
  return retval;
}

int make_call(int function, int argument)
{
  gc_push(function);
  gc_push(argument);
  int retval = cell();
  cells[retval].type = CALL;
  cells[retval].call.function = function;
  cells[retval].call.argument = argument;
  gc_pop(2);
  return retval;
}

int make_proc(int function, int environment)
{
  gc_push(function);
  gc_push(environment);
  int retval = cell();
  cells[retval].type = PROC;
  cells[retval].proc.function = function;
  cells[retval].proc.environment = environment;
  gc_pop(2);
  return retval;
}

int make_wrap(int unwrap, int environment)
{
  gc_push(unwrap);
  gc_push(environment);
  int retval = cell();
  cells[retval].type = WRAP;
  cells[retval].wrap.unwrap = unwrap;
  cells[retval].wrap.environment = environment;
  gc_pop(2);
  return retval;
}

int make_input(FILE *file)
{
  int retval = cell();
  cells[retval].type = INPUT;
  cells[retval].input.file = file;
  cells[retval].input.used = retval;
  return retval;
}

int make_output(FILE *file)
{
  int retval = cell();
  cells[retval].type = OUTPUT;
  cells[retval].output.file = file;
  cells[retval].output.used = retval;
  return retval;
}

int make_false(void) { return make_lambda(make_lambda(make_variable(0))); }

int is_false(int expression)
{
  return
    has_function(expression) &&
    has_function(function(expression)) &&
    is_variable(function(function(expression))) &&
    variable(function(function(expression))) == 0;
}

int make_true(void) { return make_lambda(make_lambda(make_variable(1))); }

int is_true(int expression)
{
  return
    has_function(expression) &&
    has_function(function(expression)) &&
    is_variable(function(function(expression))) &&
    variable(function(function(expression))) == 1;
}

int num_to_list(int number)
{
  int retval;
  if (number == 0)
    retval = make_false();
  else if (number & 0x1) {
    retval = make_pair(gc_push(make_true()), gc_push(num_to_list(number >> 1)));
    gc_pop(2);
  } else {
    retval = make_pair(gc_push(make_false()), gc_push(num_to_list(number >> 1)));
    gc_pop(2);
  };
  return retval;
}

int is_pair(int expression);

int list_to_num(int list)
{
  int retval;
  if (is_pair(list)) {
    if (is_true(first(list)))
      retval = 1 + (list_to_num(rest(list)) << 1);
    else
      retval = list_to_num(rest(list)) << 1;
  } else
    retval = 0;
  return retval;
}

int read_char(int input)
{
  int retval;
  if (used(input) != input)
    retval = used(input);
  else {
    int num = fgetc(file(gc_push(input)));
    if (num == EOF)
      retval = make_false();
    else {
      retval = make_pair(gc_push(num_to_list(num)), gc_push(make_input(file(input))));
      gc_pop(2);
    };
    cells[input].input.used = retval;
    gc_pop(1);
  }
  return retval;
}

int read_bit(int input)
{
  int retval;
  int list = gc_push(read_char(input));
  if (!is_pair(list))
    retval = list;
  else {
    int c = list_to_num(first(list));
    switch (c) {
    case '0':
      retval = make_pair(gc_push(make_false()), rest(list));
      gc_pop(1);
      break;
    case '1':
      retval = make_pair(gc_push(make_true()), rest(list));
      gc_pop(1);
      break;
    default:
      retval = read_bit(rest(list));
    };
  };
  gc_pop(1);
  return retval;
}

int write_char(int output, int list)
{
  int retval;
  gc_push(output);
  gc_push(list);
  if (used(output) != output) {
    fprintf(stderr, "Output object already used!\n");
    exit(1);
  } else {
    fputc(list_to_num(list), file(output));
    retval = make_output(file(output));
    cells[output].output.used = retval;
  }
  gc_pop(2);
  return retval;
}

int write_bit(int output, int bit)
{
  int retval;
  gc_push(output);
  gc_push(bit);
  if (is_true(bit))
    retval = write_char(output, num_to_list('1'));
  else
    retval = write_char(output, num_to_list('0'));
  gc_pop(2);
  return retval;
}

int write_false(int output)
{
  int retval;
  gc_push(output);
  retval = write_bit(output, make_false());
  gc_pop(1);
  return retval;
}

int write_true(int output)
{
  int retval;
  gc_push(output);
  retval = write_bit(output, make_true());
  gc_pop(1);
  return retval;
}

int make_pair(int first, int rest)
{
  gc_push(first);
  gc_push(rest);
  int retval = make_lambda(make_call(make_call(make_variable(0), first), rest));
  gc_pop(2);
  return retval;
}

int is_pair(int expression)
{
  return
    has_function(expression) &&
    has_function(function(expression)) &&
    has_function(function(function(expression))) &&
    is_variable(function(function(function(expression)))) &&
    variable(function(function(function(expression)))) == 0;
}

int first(int list)
{
  int retval;
  if (is_input(list))
    retval = first(read_bit(list));
  else
    retval = argument(function(function(list)));
  return retval;
}

int rest(int list)
{
  int retval;
  if (is_input(list))
    retval = rest(read_bit(list));
  else
    retval = argument(function(list));
  return retval;
}

int read_variable(int input)
{
  int retval;
  gc_push(input);
  if (is_true(first(input))) {
    retval = read_variable(rest(input));
    cells[first(retval)].variable++;
  } else
    retval = make_pair(make_variable(0), rest(input));
  gc_pop(1);
  return retval;
}

int read_lambda(int input)
{
  int retval;
  int term = gc_push(read_expression(input));
  retval = make_pair(make_lambda(first(term)), rest(term));
  gc_pop(1);
  return retval;
}

int read_call(int input)
{
  int retval;
  int function = gc_push(read_expression(input));
  int argument = gc_push(read_expression(rest(function)));
  retval = make_pair(make_call(first(function), first(argument)), rest(argument));
  gc_pop(2);
  return retval;
}

int read_expression(int input)
{
  int retval;
  gc_push(input);
  if (is_false(first(input))) {
    int b2 = rest(input);
    if (is_false(first(b2)))
      retval = read_lambda(rest(b2));
    else if (is_true(first(b2)))
      retval = read_call(rest(b2));
    else {
      fprintf(stderr, "Incomplete expression!\n");
      exit(1);
    };
  } else if (is_true(first(input)))
    retval = read_variable(rest(input));
  else {
    fprintf(stderr, "Incomplete expression!\n");
    exit(1);
  };
  gc_pop(1);
  return retval;
}

int length(int list)
{
  int retval;
  if (!is_pair(list))
    retval = 0;
  else
    retval = 1 + length(rest(list));
  return retval;
}

int lookup(int variable, int environment)
{
  int retval;
  if (is_pair(environment)) {
    if (variable > 0)
      retval = lookup(variable - 1, rest(environment));
    else
      retval = first(environment);
  } else
    retval = MAX_CELLS;
  return retval;
}

int write_variable(int output, int variable)
{
  int retval;
  gc_push(output);
  gc_push(variable);
  int rest = write_true(output);
  if (variable > 0)
    retval = write_variable(rest, variable - 1);
  else
    retval = write_false(rest);
  gc_pop(2);
  return retval;
}

int normalise(int expression, int local_environment, int local_depth, int depth)
{
  int retval;
  gc_push(expression);
  gc_push(local_environment);
  switch (type(expression)) {
  case VARIABLE:
    if (variable(expression) < local_depth)
      retval = expression;
    else {
      int value = lookup(variable(expression) - local_depth, local_environment);
      if (value != MAX_CELLS)
        retval = normalise(value, local_environment, local_depth, depth);
      else
        retval = make_variable(variable(expression) + depth - local_depth);
    };
    break;
  case LAMBDA:
    retval = make_lambda(normalise(function(expression), local_environment, local_depth + 1, depth + 1));
    break;
  case CALL: {
    int fun = gc_push(normalise(function(expression), local_environment, local_depth, depth));
    int arg = gc_push(normalise(argument(expression), local_environment, local_depth, depth));
    retval = make_call(fun, arg);
    gc_pop(2);
    break; }
  case PROC:
    retval = make_lambda(normalise(function(expression), environment(expression), 1, depth + 1));
    break;
  case WRAP:
    retval = normalise(unwrap(expression), environment(expression), 0, depth);
    break;
  case INPUT:
    retval = make_variable(depth - 2);
    break;
  case OUTPUT:
    retval = make_variable(depth - 1);
    break;
  default:
    retval = expression;
  };
  gc_pop(2);
  return retval;
}

int write_expression(int output, int expression)
{
  int retval;
  gc_push(output);
  gc_push(expression);
  switch (type(expression)) {
  case VARIABLE:
    retval = write_variable(output, variable(expression));
    break;
  case LAMBDA: {
    int rest = write_false(write_false(output));
    retval = write_expression(rest, function(expression));
    break; }
  case CALL: {
    int rest = write_true(write_false(output));
    retval = write_expression(write_expression(rest, function(expression)), argument(expression));
    break; }
  default:
    fprintf(stderr, "Cannot print expression!\n");
    exit(1);
  }
  gc_pop(2);
  return retval;
}

int eval_expression(int expression, int local_environment)
{
  int retval;
  gc_push(expression);
  gc_push(local_environment);
  switch (type(expression)) {
  case VARIABLE: {
    int value = lookup(variable(expression), local_environment);
    if (value != MAX_CELLS)
      retval = eval_expression(value, local_environment);
    else
      retval = make_variable(variable(expression) - length(local_environment));
    break; }
  case LAMBDA:
    retval = make_proc(function(expression), local_environment);
    break;
  case CALL: {
    int eval_fun = gc_push(eval_expression(function(expression), local_environment));
    if (is_proc(eval_fun)) {
      int wrap_argument = make_wrap(argument(expression), local_environment);
      int call_environment = gc_push(make_pair(wrap_argument, environment(eval_fun)));
      retval = eval_expression(function(eval_fun), call_environment);
      gc_pop(1);
    } else if (is_input(eval_fun)) {
      int bit = eval_expression(read_bit(eval_fun), local_environment);
      retval = eval_expression(make_call(bit, argument(expression)), local_environment);
    } else if (is_output(eval_fun))
      retval = write_bit(eval_fun, eval_expression(argument(expression), local_environment));
    else
      retval = eval_fun;
    gc_pop(1);
    break; }
  case WRAP:
    retval = eval_expression(unwrap(expression), environment(expression));
    break;
  default:
    retval = expression;
  }
  gc_pop(2);
  return retval;
}
