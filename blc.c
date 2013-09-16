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
#include <stdio.h>
#include "blc.h"

#define NVERBOSE

#define MAX_CELLS 1048576

typedef struct { int fun; int arg; } call_t;

typedef struct { int term; int body; } definition_t;

typedef struct { int term; int stack; } proc_t;

typedef struct { int unwrap; int context; int cache; } wrap_t;

typedef struct { FILE *file; int used; } input_t;

typedef struct {
  type_t type;
  union {
    int idx;
    int body;
    call_t call;
    proc_t proc;
    wrap_t wrap;
    int target;
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

int is_var(int cell) { return type(cell) == VAR; }
int is_lambda(int cell) { return type(cell) == LAMBDA; }
int is_call(int cell) { return type(cell) == CALL; }
int is_proc(int cell) { return type(cell) == PROC; }
int is_wrap(int cell) { return type(cell) == WRAP; }
int is_memoize(int cell) { return type(cell) == MEMOIZE; }
int is_input(int cell) { return type(cell) == INPUT; }
int is_output(int cell) { return type(cell) == OUTPUT; }

int idx(int cell) { assert(is_var(cell)); return cells[cell].idx; }
int body(int cell) { assert(is_lambda(cell)); return cells[cell].body; }
int fun(int cell) { assert(is_call(cell)); return cells[cell].call.fun; }
int arg(int cell) { assert(is_call(cell)); return cells[cell].call.arg; }
int term(int cell) { assert(is_proc(cell)); return cells[cell].proc.term; }
int stack(int cell) { assert(is_proc(cell)); return cells[cell].proc.stack; }
int unwrap(int cell) { assert(is_wrap(cell)); return cells[cell].wrap.unwrap; }
int context(int cell) { assert(is_wrap(cell)); return cells[cell].wrap.context; }
int cache(int cell) { assert(is_wrap(cell)); return cells[cell].wrap.cache; }
int target(int cell) { assert(is_memoize(cell)); return cells[cell].target; }
FILE *file(int cell) { assert(is_input(cell)); return cells[cell].input.file; }
int used(int cell) { assert(is_input(cell)); return cells[cell].input.used; }

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

int is_f_(int cell)
{
  return is_proc(cell) &&
         is_proc(term(cell)) &&
         is_var(term(term(cell))) &&
         idx(term(term(cell))) == 0;
}

int op_if(int condition, int consequent, int alternative)
{
  return call(call(condition, consequent), alternative);
}

int pair_ = -1;

int pair(int first, int rest)
{
  return call(call(pair_, rest), first);
}

int first_(int list) { return arg(list); }
int rest_(int list) { return arg(fun(list)); }

int at_(int list, int i)
{
  if (is_f_(list)) {
    fputs("Array out of range!\n", stderr);
    exit(1);
  };
  return i > 0 ? at_(rest_(list), i - 1) : first_(list);
}

int proc_stack(int term, int stack)
{
  int retval = cell(PROC);
  cells[retval].proc.term = term;
  cells[retval].proc.stack = stack;
  return retval;
}

int proc(int term) { return proc_stack(term, f_); }

int wrap(int unwrap, int context)
{
  int retval = cell(WRAP);
  cells[retval].wrap.unwrap = unwrap;
  cells[retval].wrap.context = context;
  cells[retval].wrap.cache = retval;
  return retval;
}

/*
void memoize(int cell, int value)
{
  assert(is_wrap(cell));
  cells[cell].wrap.cache = value;
}
*/

int memoize(int target)
{
  int retval = cell(MEMOIZE);
  cells[retval].target = target;
  return retval;
}

int input(FILE *file)
{
  int retval = cell(INPUT);
  cells[retval].input.file = file;
  cells[retval].input.used = retval;
  return retval;
}

int output_ = -1;
int output(void) { return output_; }

int read_char(int in)
{
  int retval;
  if (used(in) != in)
    retval = used(in);
  else {
    int c = fgetc(file(in));
    if (c == EOF)
      retval = f_;
    else
      retval = pair(int_to_num(c), input(file(in)));
    cells[in].input.used = retval;
  }
  return retval;
}

#ifndef NVERBOSE
void display(int cell);
#endif

int eval_env(int cell, int env, int cont) // cont: λx.(return x)
{
  int retval;
  int quit = 0;
  while (!quit) {
    switch (type(cell)) {
    case VAR:
      cell = at_(env, idx(cell));
      break;
    case LAMBDA:
      cell = proc_stack(body(cell), env);
      break;
    case CALL: {
      int f = fun(cell);
      if (is_proc(f)) {
        env = pair(wrap(arg(cell), env), stack(f));
        cell = term(f);
      } else if (is_input(f))
        cell = call(read_char(f), arg(cell));
      else
        cell = call(eval_env(f, env, cont), arg(cell)); // (cell λf.((f wrap_arg) cont))
      break; }
    case WRAP:
      if (cache(cell) != cell)
        cell = cache(cell);
      else {
        cont = lambda(call(memoize(cell), cont));
        int value = eval_env(unwrap(cell), context(cell), cont); // (cell λx.((memo cell x) cont))
        cells[target(fun(body(cont)))].wrap.cache = value;
        cont = arg(body(cont));
      };
      break;
    default:
      if (is_output(cont)) {
        retval = cell;// (cell cont)
        quit = 1;
      } else {
        retval = cell;
        quit = 1;
      };
    };
  };
  return retval;
}

int eval(int cell) { return eval_env(cell, f_, output_); }

int is_f(int cell) { return eval(op_if(cell, t_, f_)) == f_; }

int first(int list) { return call(list, t_); }
int rest(int list) { return call(list, f_); }
int empty(int list) { return call(call(list, proc(lambda(lambda(f_)))), t_); }
int at(int list, int i) { return i > 0 ? at(rest(list), i - 1) : first(list); }

int op_not(int a) { return op_if(a, f_, t_); }
int op_and(int a, int b) { return op_if(a, b, f_); }
int op_or(int a, int b) { return op_if(a, t_, b); }
int eq_bool_ = -1;
int eq_bool(int a, int b) { return op_if(eq_bool_, a, b); }

int int_to_num(int integer)
{
  assert(integer >= 0);
  return integer == 0 ? f_ : pair(integer & 0x1 ? t_ : f_, int_to_num(integer >> 1));
}

int num_to_int_(int number)
{
  return is_f_(number) ? 0 : (is_f_(first_(number)) ? 0 : 1) | num_to_int_(rest_(number)) << 1;
}

int num_to_int(int number)
{
  int eval_num = eval(number);
  return !is_f(empty(eval_num)) ? 0 : (is_f(first(eval_num)) ? 0 : 1) | num_to_int(rest(eval_num)) << 1;
}

int y_ = -1;

int y_comb(int fun) { return call(y_, proc(fun)); }

int str_to_list(const char *str)
{
  return *str == '\0' ? f_ : pair(int_to_num(*str), str_to_list(str + 1));
}

#define BUFSIZE 1024
char buffer[BUFSIZE];

char *list_to_buffer_(int list, char *buffer, int bufsize)
{
  if (bufsize <= 1) {
    fputs("Buffer too small!\n", stderr);
    exit(1);
  };
  if (is_f_(list))
    *buffer = '\0';
  else {
    *buffer = num_to_int_(first_(list));
    list_to_buffer_(rest_(list), buffer + 1, bufsize - 1);
  };
  return buffer;
}

char *list_to_str_(int list) { return list_to_buffer_(list, buffer, BUFSIZE); }

char *list_to_buffer(int list, char *buffer, int bufsize)
{
  if (bufsize <= 1) {
    fputs("Buffer too small!\n", stderr);
    exit(1);
  };
  int eval_list = eval(list);
  if (!is_f(empty(eval_list)))
    *buffer = '\0';
  else {
    *buffer = num_to_int(first(eval_list));
    list_to_buffer(rest(list), buffer + 1, bufsize - 1);
  };
  return buffer;
}

char *list_to_str(int list) { return list_to_buffer(list, buffer, BUFSIZE); }

int eq_num_ = -1;
int eq_num(int a, int b) { return call(call(eq_num_, a), b); }

int id_ = -1;
int id(void) { return id_; }

int map_ = -1;
int map(int list, int fun) { return call(call(map_, fun), list); }

int select_if_ = -1;
int select_if(int list, int fun) { return call(call(select_if_, fun), list); }

int bits_to_bytes_ = -1;
int bits_to_bytes(int bits) { return call(bits_to_bytes_, bits); }

int bytes_to_bits_ = -1;
int bytes_to_bits(int bytes) { return call(bytes_to_bits_, bytes); }

int select_binary_ = -1;
int select_binary(int list) { return call(select_binary_, list); }

void init(void)
{
  f_ = cell(PROC);
  cells[f_].proc.term = proc(var(0));
  cells[f_].proc.stack = f_;
  t_ = proc(lambda(var(1)));
  output_ = cell(OUTPUT);
  pair_ = proc(lambda(lambda(op_if(var(0), var(1), var(2)))));
  eq_bool_ = proc(lambda(op_if(var(0), var(1), op_not(var(1)))));
  y_ = proc(call(lambda(call(var(1), call(var(0), var(0)))),
                 lambda(call(var(1), call(var(0), var(0))))));
  eq_num_ = y_comb(lambda(lambda(op_if(op_or(empty(var(0)), empty(var(1))),
                                       op_and(empty(var(0)), empty(var(1))),
                                       op_and(eq_bool(first(var(0)), first(var(1))),
                                              call(call(var(2), rest(var(0))), rest(var(1))))))));
  id_ = proc(var(0));
  map_ = y_comb(lambda(lambda(op_if(empty(var(0)),
                                    f_,
                                    pair(call(var(1), first(var(0))),
                                         call(call(var(2), var(1)), rest(var(0))))))));
  select_if_ = y_comb(lambda(lambda(op_if(empty(var(0)),
                                          f_,
                                          op_if(call(var(1), first(var(0))),
                                                pair(first(var(0)),
                                                     call(call(var(2), var(1)), rest(var(0)))),
                                                call(call(var(2), var(1)), rest(var(0))))))));
  bits_to_bytes_ = proc(map(var(0), proc(op_if(var(0), int_to_num('1'), int_to_num('0')))));
  bytes_to_bits_ = proc(map(var(0), proc(eq_num(var(0), int_to_num('1')))));
  select_binary_ = proc(select_if(var(0), proc(op_or(eq_num(var(0), int_to_num('0')),
                                                     eq_num(var(0), int_to_num('1'))))));
}

#ifndef NVERBOSE
void display(int cell)
{
  typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, INPUT } type_t;
  if (cell == t_)
    fputs("true", stderr);
  else if (cell == f_)
    fputs("false", stderr);
  else if (cell == pair_)
    fputs("pair", stderr);
  else
    switch (type(cell)) {
    case VAR:
      fprintf(stderr, "var(%d)", idx(cell));
      break;
    case LAMBDA:
      fputs("lambda(", stderr);
      display(body(cell));
      fputs(")", stderr);
      break;
    case CALL:
      fputs("call(", stderr);
      display(fun(cell));
      fputs(", ", stderr);
      display(arg(cell));
      fputs(")", stderr);
      break;
    case PROC:
      fputs("proc(", stderr);
      display(term(cell));
      fputs(")", stderr);
      break;
    case WRAP:
      fputs("wrap(", stderr);
      display(unwrap(cell));
      fputs(")", stderr);
      break;
    case INPUT:
      fputs("input(...)", stderr);
      break;
    };
}
#endif

FILE *tmp_ = NULL;

void destroy(void)
{
  if (tmp_) fclose(tmp_);
  tmp_ = NULL;
}

int str_to_input(const char *text)
{
  if (tmp_) fclose(tmp_);
  FILE *f = fopen("test.tmp", "w");
  fputs(text, f);
  fclose(f);
  tmp_ = fopen("test.tmp", "r");
  return input(tmp_);
}

int read_var(int in)
{
  int retval;
  int eval_in = eval(in);
  if (!is_f(empty(eval_in))) {
    fputs("Incomplete variable!\n", stderr);
    exit(1);
  } else
    if (is_f(first(eval_in)))
      retval = pair(var(0), rest(eval_in));
    else {
      retval = read_var(rest(eval_in));
      cells[first_(retval)].idx++;
    };
  return retval;
}

int read_lambda(int in)
{
  int term = read_expr(in);
  return pair(lambda(first_(term)), rest_(term));
}

int read_call(int in)
{
  int fun = read_expr(in);
  int arg = read_expr(rest_(fun));
  return pair(call(first_(fun), first_(arg)), rest_(arg));
}

int read_expr(int in)
{
  int retval;
  int eval_in = eval(in);
  if (!is_f(empty(eval_in))) {
    fputs("Incomplete expression!\n", stderr);
    exit(1);
  } else {
    if (is_f(first(eval_in))) {
      int eval_rest = eval(rest(eval_in));
      if (!is_f(empty(eval_rest))) {
        fputs("Incomplete structure!\n", stderr);
        exit(1);
      };
      if (is_f(first(eval_rest)))
        retval = read_lambda(rest(eval_rest));
      else
        retval = read_call(rest(eval_rest));
    } else
      retval = read_var(rest(eval_in));
  };
  return retval;
}

void write_expression(int expr, int env, FILE *stream)
{
  int list = eval_env(bits_to_bytes(expr), env, output_);
  while (is_f(empty(list))) {
    fputc(num_to_int(first(list)), stream);
    list = eval(rest(list));
  };
  fputc('\n', stream);
}

