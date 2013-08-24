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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_CELLS 1000
typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, INPUT } type_t;

typedef struct { int fun; int arg; } call_t;

typedef struct { int term; int body; } definition_t;

typedef struct { int term; int stack; } proc_t;

typedef struct { int unwrap; int env; } wrap_t;

typedef struct { FILE *file; int used; } input_t;

typedef struct {
  type_t type;
  union {
    int idx;
    int body;
    call_t call;
    definition_t definition;
    proc_t proc;
    wrap_t wrap;
    input_t input;
  };
} cell_t;

cell_t cells[MAX_CELLS];
int n_cells = 0;

int cell(int type)
{
  if (n_cells >= MAX_CELLS) {
    fputs("Out of memory!\n", stderr);
    exit(1);
  };
  int retval = n_cells++;
  cells[retval].type = type;
  return retval;
}

void check_cell(int cell) { assert(cell >= 0 && cell < MAX_CELLS); }

int type(int cell) { check_cell(cell); return cells[cell].type; }
int is_type(int cell, int t) { return type(cell) == t; }
void check_type(int cell, int t) { assert(is_type(cell, t)); }

int is_var(int cell) { return is_type(cell, VAR); }
int is_lambda(int cell) { return is_type(cell, LAMBDA); }
int is_call(int cell) { return is_type(cell, CALL); }
int is_proc(int cell) { return is_type(cell, PROC); }
int is_wrap(int cell) { return is_type(cell, WRAP); }
int is_input(int cell) { return is_type(cell, INPUT); }

int idx(int cell) { check_type(cell, VAR); return cells[cell].idx; }
int body(int cell) { check_type(cell, LAMBDA); return cells[cell].body; }
int fun(int cell) { check_type(cell, CALL); return cells[cell].call.fun; }
int arg(int cell) { check_type(cell, CALL); return cells[cell].call.arg; }
int term(int cell) { check_type(cell, PROC); return cells[cell].proc.term; }
int stack(int cell) { check_type(cell, PROC); return cells[cell].proc.stack; }

int var(int idx)
{
  int retval = cell(VAR);
  cells[retval].idx = idx;
  return retval;
}

int lambda(int body)
{
  int retval = cell(LAMBDA);
  cells[retval].body = body;
  return retval;
}

int call(int fun, int arg)
{
  int retval = cell(CALL);
  cells[retval].call.fun = fun;
  cells[retval].call.arg = arg;
  return retval;
}

int f_ = -1;
int t_ = -1;

int f(void) { return f_; }
int t(void) { return t_; }

int is_f(int cell)
{
  return
    (is_lambda(cell) &&
     is_lambda(body(cell)) &&
     is_var(body(body(cell))) &&
     idx(body(body(cell))) == 0) ||
    (is_proc(cell) &&
     is_lambda(term(cell)) &&
     is_var(body(term(cell))) &&
     idx(body(term(cell))) == 0);
}

int if_(int condition, int consequent, int alternative)
{
  return call(call(condition, consequent), alternative);
}

int pair_ = -1;

int pair(int first, int rest)
{
  return call(call(pair_, first), rest);
}

int first(int list) { return arg(fun(list)); }
int rest(int list) { return arg(list); }

int at(int list, int i)
{
  int retval;
  if (is_f(list)) {
    fputs("Array out of range!\n", stderr);
    exit(1);
  } else {
    if (i > 0)
      retval = at(rest(list), i - 1);
    else
      retval = first(list);
  };
  return retval;
}

int proc(int term, int stack)
{
  int retval = cell(PROC);
  cells[retval].proc.term = term;
  cells[retval].proc.stack = stack;
  return retval;
}

int eval(int cell, int env)
{
  int retval;
  switch (type(cell)) {
    case VAR:
      retval = eval(at(env, idx(cell)), env);
      break;
    case LAMBDA:
      retval = proc(body(cell), env);
      break;
    default:
      retval = cell;
  };
  return retval;
}

void init(void)
{
  f_ = lambda(lambda(var(0)));
  t_ = lambda(lambda(var(1)));
  pair_ = lambda(lambda(lambda(if_(var(0), var(1), var(2)))));
}

int main(void)
{
  init();
  // variable
  assert(type(var(0)) == VAR);
  assert(is_type(var(0), VAR));
  assert(is_var(var(0)));
  assert(idx(var(1)) == 1);
  // lambda (function)
  assert(type(lambda(var(0))) == LAMBDA);
  assert(is_type(lambda(var(0)), LAMBDA));
  assert(is_lambda(lambda(var(0))));
  assert(idx(body(lambda(var(1)))) == 1);
  // call
  assert(type(call(lambda(var(0)), var(0))) == CALL);
  assert(is_type(call(lambda(var(0)), var(0)), CALL));
  assert(is_call(call(lambda(var(0)), var(0))));
  assert(idx(body(fun(call(lambda(var(1)), var(2))))) == 1);
  assert(idx(arg(call(lambda(var(1)), var(2)))) == 2);
  // false and true
  assert(idx(body(body(f()))) == 0);
  assert(idx(body(body(t()))) == 1);
  assert(is_f(f()));
  assert(!is_f(t()));
  // conditional
  assert(idx(fun(fun(if_(var(1), var(2), var(3))))) == 1);
  assert(idx(arg(fun(if_(var(1), var(2), var(3))))) == 2);
  assert(idx(arg(if_(var(1), var(2), var(3)))) == 3);
  // lists (pairs)
  assert(!is_f(pair(t(), f())));
  assert(idx(first(pair(var(1), f()))) == 1);
  assert(is_f(rest(pair(var(1), f()))));
  assert(idx(at(pair(var(1), pair(var(2), pair(var(3), f()))), 0)) == 1);
  assert(idx(at(pair(var(1), pair(var(2), pair(var(3), f()))), 1)) == 2);
  assert(idx(at(pair(var(1), pair(var(2), pair(var(3), f()))), 2)) == 3);
  // evaluation of variables
  assert(is_f(eval(var(0), pair(f(), f()))));
  assert(is_f(eval(var(1), pair(t(), pair(f(), f())))));
  // procs (closures)
  assert(type(proc(lambda(var(0)), f())) == PROC);
  assert(is_type(proc(lambda(var(0)), f()), PROC));
  assert(is_proc(proc(lambda(var(0)), f())));
  assert(idx(term(proc(var(0), f()))) == 0);
  assert(is_f(stack(proc(var(0), f()))));
  // evaluation of lambdas
  assert(is_proc(eval(lambda(var(0)), f())));
  assert(idx(term(eval(lambda(var(1)), f()))) == 1);
  assert(is_f(stack(eval(lambda(var(0)), f()))));
  return 0;
}
