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

#define MAX_CELLS 16384
#define MAX_REGISTERS 65536

typedef enum { VARIABLE, LAMBDA, CALL, PROC, DEFINITION, WRAP, INPUT, OUTPUT } type_t;

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

int is_nil(int cell) { return cell == NIL; }

int type(int cell) { return cells[cell].type; }

int is_type(int cell, int t) { return is_nil(cell) ? 0 : type(cell) == t; }

int is_variable(int cell) { return is_type(cell, VARIABLE); }
int is_lambda(int cell) { return is_type(cell, LAMBDA); }
int is_call(int cell) { return is_type(cell, CALL); }
int is_definition(int cell) { return is_type(cell, DEFINITION); }
int is_proc(int cell) { return is_type(cell, PROC); }
int is_wrap(int cell) { return is_type(cell, WRAP); }
int is_input(int cell) { return is_type(cell, INPUT); }
int is_output(int cell) { return is_type(cell, OUTPUT); }

int variable(int cell) { return is_variable(cell) ? cells[cell].variable : NIL; }
int function(int cell) { return is_call(cell) ? cells[cell].call.function : is_proc(cell) ? cells[cell].proc.function : is_lambda(cell) ? cells[cell].lambda : NIL; }
int argument(int cell) { return is_call(cell) ? cells[cell].call.argument : NIL; }
int term(int cell) { return is_definition(cell) ? cells[cell].definition.term : NIL; }
int body(int cell) { return is_definition(cell) ? cells[cell].definition.body : NIL; }
int unwrap(int cell) { return is_wrap(cell) ? cells[cell].wrap.unwrap : NIL; }
int environment(int cell) { return is_proc(cell) ? cells[cell].proc.environment : is_wrap(cell) ? cells[cell].wrap.environment : NIL; }
FILE *file(int cell) { return is_input(cell) ? cells[cell].input.file : is_output(cell) ? cells[cell].output.file : NULL; }
int used(int cell) { return is_input(cell) ? cells[cell].input.used : is_output(cell) ? cells[cell].output.used : NIL; }

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
  if (!is_nil(expression))
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
        case DEFINITION:
          mark(term(expression));
          mark(body(expression));
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

int make_definition(int term, int body)
{
  int retval;
  if (!is_nil(term) && !is_nil(body)) {
    gc_push(term);
    gc_push(body);
    retval = cell();
    if (!is_nil(retval)) {
      cells[retval].type = DEFINITION;
      cells[retval].definition.term = term;
      cells[retval].definition.body = body;
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
    cells[retval].input.used = NIL;
  };
  return retval;
}

int make_output(FILE *file)
{
  int retval = cell();
  if (!is_nil(retval)) {
    cells[retval].type = OUTPUT;
    cells[retval].output.file = file;
    cells[retval].output.used = NIL;
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
  if (is_nil(input))
    retval = NIL;
  else if (!is_nil(cells[input].input.used))
    retval = cells[input].input.used;
  else {
    int c = fgetc(file(gc_push(input)));
    switch (c) {
    case '0':
      retval = make_pair(gc_push(make_false()), gc_push(make_input(file(input))));
      gc_pop(2);
      break;
    case '1':
      retval = make_pair(gc_push(make_true()), gc_push(make_input(file(input))));
      gc_pop(2);
      break;
    case EOF:
      retval = make_false();
      break;
    default:
      retval = read_bit(input);
    };
    cells[input].input.used = retval;
    gc_pop(1);
  };
  return retval;
}

int write_bit(int output, int bit)
{
  int retval;
  if (!is_nil(cells[output].output.used))
    retval = NIL;
  else if (!is_nil(bit)) {
    gc_push(output);
    if (is_false(bit)) {
      fputc('0', file(output));
      retval = make_output(file(output));
    } else if (is_true(bit)) {
      fputc('1', file(output));
      retval = make_output(file(output));
    } else
      retval = NIL;
    cells[output].output.used = retval;
    gc_pop(1);
  } else
    retval = NIL;
  return retval;
}

int make_pair(int first, int rest)
{
  int retval;
  if (!is_nil(first) && !is_nil(rest)) {
    gc_push(first);
    gc_push(rest);
    retval = make_lambda(make_call(make_call(make_variable(0), first), rest));
    gc_pop(2);
  } else
    retval = NIL;
  return retval;
}

int is_pair(int expression) { return variable(function(function(function(expression)))) == 0; }

int first(int list) { return argument(function(function(list))); }

int rest(int list) { return argument(function(list)); }

int read_variable(int input)
{
  int retval;
  int b = gc_push(read_bit(gc_push(input)));
  if (is_false(first(b)))
    retval = make_pair(make_variable(0), rest(b));
  else if (is_true(first(b))) {
    retval = read_variable(rest(b));
    if (!is_nil(retval)) cells[first(retval)].variable++;
  } else
    retval = NIL;
  gc_pop(2);
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

int read_definition(int input)
{
  int retval;
  int term = gc_push(read_expression(input));
  int body = gc_push(read_expression(rest(term)));
  retval = make_pair(make_definition(first(term), first(body)), rest(body));
  gc_pop(2);
  return retval;
}

int read_expression(int input)
{
  int retval;
  int b1 = read_bit(gc_push(input));
  if (is_false(first(b1))) {
    int b2 = read_bit(rest(b1));
    if (is_false(first(b2)))
      retval = read_lambda(rest(b2));
    else if (is_true(first(b2))) {
      int b3 = read_bit(rest(b2));
      if (is_true(first(b3)))
        retval = read_call(rest(b3));
      else
        retval = read_definition(rest(b3));
    } else
      retval = NIL;
  } else if (is_true(first(b1)))
    retval = read_variable(rest(b1));
  else
    retval = NIL;
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
  return variable > 0 ? lookup(variable - 1, rest(environment)) : first(environment);
}

void print_variable(int variable, FILE *file)
{
  fputc('1', file);
  if (variable > 0)
    print_variable(variable - 1, file);
  else
    fputc('0', file);
}

int normalise(int expression, int local_environment, int local_depth, int depth)
{
  int retval;
  gc_push(expression);
  gc_push(local_environment);
  if (!is_nil(expression)) {
    switch (type(expression)) {
    case VARIABLE:
      if (variable(expression) < local_depth)
        retval = expression;
      else {
        int value = lookup(variable(expression) - local_depth, local_environment);
        if (!is_nil(value))
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
    case DEFINITION: {
      int def = gc_push(normalise(term(expression), local_environment, local_depth, depth));
      int expr = gc_push(normalise(body(expression), local_environment, local_depth + 1, depth + 1));
      retval = make_definition(def, expr);
      gc_pop(2);
      break; }
    case PROC:
      retval = make_lambda(normalise(function(expression), environment(expression), 1, depth + 1));
      break;
    case WRAP:
      retval = normalise(unwrap(expression), environment(expression), 0, depth);
      break;
    case INPUT:
      retval = expression;
      break;
    case OUTPUT:
      retval = expression;
      break;
    default:
      retval = NIL;
    }
  } else
    retval = NIL;
  gc_pop(2);
  return retval;
}

void print_expression(int expression, FILE *file)
{
  if (!is_nil(expression)) {
    switch (type(expression)) {
    case VARIABLE:
      print_variable(variable(expression), file);
      break;
    case LAMBDA:
      fputs("00", file);
      print_expression(function(expression), file);
      break;
    case CALL:
      fputs("011", file);
      print_expression(function(expression), file);
      print_expression(argument(expression), file);
      break;
    case DEFINITION:
      fputs("010", file);
      print_expression(term(expression), file);
      print_expression(body(expression), file); // test?
      break;
    case INPUT:
      if (is_nil(used(expression)))
        fputs("#<input>", file);
      else
        fputs("#<input(used)>", file);
      break;
    case OUTPUT:
      if (is_nil(used(expression)))
        fputs("#<output>", file);
      else
        fputs("#<output(used)>", file);
      break;
    }
  } else
    fputs("#<err>", file);
}

int eval_expression(int expression, int local_environment)
{
  int retval;
  gc_push(expression);
  gc_push(local_environment);
  if (!is_nil(expression) && !is_nil(local_environment)) {
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
    case DEFINITION: {
      int wrap_term = gc_push(make_wrap(term(expression), local_environment));
      int body_environment = gc_push(make_pair(wrap_term, local_environment));
      retval = eval_expression(body(expression), body_environment);
      gc_pop(2);
      break; }
    case WRAP:
      retval = eval_expression(unwrap(expression), environment(expression));
      break;
    case PROC:
    case INPUT:
    case OUTPUT:
      retval = expression;
      break;
    default:
      retval = NIL;
    }
  } else
    retval = NIL;
  gc_pop(2);
  return retval;
}
