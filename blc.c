/* BLC - Binary Lambda Calculus interpreter
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

#define MAX_CELLS 4000000

typedef struct { int fun; int arg; } call_t;

typedef struct { int term; int stack; } proc_t;

typedef struct { int unwrap; int context; int cache; } wrap_t;

typedef struct { int value; int target; } memoize_t;

typedef struct { FILE *file; int used; } input_t;

typedef struct {
  type_t type;
  union {
    int idx;
    int body;
    call_t call;
    proc_t proc;
    wrap_t wrap;
    memoize_t memoize;
    input_t input;
  };
#ifndef NDEBUG
  const char *tag;
#endif
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

#ifndef NDEBUG
int tag(int cell, const char *value)
{
  cells[cell].tag = value;
  return cell;
}
#endif

static void check_cell(int cell) { assert(cell >= 0 && cell < MAX_CELLS); }

int type(int cell) { check_cell(cell); return cells[cell].type; }

int is_type(int cell, int t) { return type(cell) == t; }

int idx(int cell) { assert(is_type(cell, VAR)); return cells[cell].idx; }
int body(int cell) { assert(is_type(cell, LAMBDA)); return cells[cell].body; }
int fun(int cell) { assert(is_type(cell, CALL)); return cells[cell].call.fun; }
int arg(int cell) { assert(is_type(cell, CALL)); return cells[cell].call.arg; }
int term(int cell) { assert(is_type(cell, PROC)); return cells[cell].proc.term; }
int stack(int cell) { assert(is_type(cell, PROC)); return cells[cell].proc.stack; }
int unwrap(int cell) { assert(is_type(cell, WRAP)); return cells[cell].wrap.unwrap; }
int context(int cell) { assert(is_type(cell, WRAP)); return cells[cell].wrap.context; }
int cache(int cell) { assert(is_type(cell, WRAP)); return cells[cell].wrap.cache; }
int value(int cell) { assert(is_type(cell, MEMOIZE)); return cells[cell].memoize.value; }
int target(int cell) { assert(is_type(cell, MEMOIZE)); return cells[cell].memoize.target; }
FILE *file(int cell) { assert(is_type(cell, INPUT)); return cells[cell].input.file; }
int used(int cell) { assert(is_type(cell, INPUT)); return cells[cell].input.used; }

int var(int idx)
{
  int retval = cell(VAR);
  cells[retval].idx = idx;
#ifndef NDEBUG
  cells[retval].tag = NULL;
#endif
  return retval;
}

int lambda(int body)
{
  int retval = cell(LAMBDA);
  cells[retval].body = body;
  return retval;
}
int lambda2(int body) { return lambda(lambda(body)); }
int lambda3(int body) { return lambda(lambda(lambda(body))); }

int call(int fun, int arg)
{
  int retval = cell(CALL);
  cells[retval].call.fun = fun;
  cells[retval].call.arg = arg;
  return retval;
}
int call2(int fun, int arg1, int arg2) { return call(call(fun, arg2), arg1); }
int call3(int fun, int arg1, int arg2, int arg3) { return call(call(call(fun, arg3), arg2), arg1); }

int f_ = -1;
int t_ = -1;

int f(void) { return f_; }
int t(void) { return t_; }

int is_f_(int cell)
{
  return is_type(cell, PROC) &&
         is_type(term(cell), PROC) &&
         is_type(term(term(cell)), VAR) &&
         idx(term(term(cell))) == 0;
}

int op_if(int condition, int consequent, int alternative)
{
  return call2(condition, alternative, consequent);
}

int pair_ = -1;
int pair(int first, int rest) { return call2(pair_, first, rest); }

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

int proc(int term) { return proc_stack(term, f()); }

int wrap(int unwrap, int context)
{
  int retval = cell(WRAP);
  cells[retval].wrap.unwrap = unwrap;
  cells[retval].wrap.context = context;
  cells[retval].wrap.cache = retval;
  return retval;
}

int store(int cell, int value)
{
  assert(is_type(cell, WRAP));
  cells[cell].wrap.cache = value;
  return value;
}

int memoize(int value, int target)
{
  int retval = cell(MEMOIZE);
  cells[retval].memoize.value = value;
  cells[retval].memoize.target = target;
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
      retval = f();
    else
      retval = pair(int_to_num(c), input(file(in)));
    cells[in].input.used = retval;
  }
  return retval;
}

#ifndef NDEBUG
void display(int cell)
{
  if (cells[cell].tag)
    fprintf(stderr, "%s", cells[cell].tag);
  else {
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
      fputs("input", stderr);
      break;
    case OUTPUT:
      fputs("output", stderr);
      break;
    default:
      assert(0);
    };
  };
}
#endif


int reindex(int cell, int index)
{
  int retval;
  switch (type(cell)) {
  case VAR:
    retval = var(idx(cell) + (idx(cell) >= index ? 1 : 0));
    break;
  case LAMBDA:
    retval = lambda(reindex(body(cell), index + 1));
    break;
  case CALL:
    retval = call(reindex(fun(cell), index), reindex(arg(cell), index));
    break;
  case PROC:
    retval = lambda(reindex(term(cell), index + 1));
    break;
  case INPUT:
    retval = cell;
    break;
  case OUTPUT:
    retval = cell;
    break;
  default:// WRAP, MEMOIZE
    assert(0);
  };
  return retval;
}

int cps(int cell, int cont)
{
  int retval;
  switch (type(cell)) {
  case VAR:
    retval = call(cont, cell);
    break;
  case LAMBDA:
    retval = call(cont, lambda(lambda(cps(reindex(body(cell), 0), var(0)))));
    break;
  case CALL:
    retval = cps(fun(cell), lambda(cps(reindex(arg(cell), 0), lambda(call(call(var(1), var(0)), reindex(reindex(cont, 0), 0))))));
    break;
  case PROC:
    retval = call(cont, lambda(lambda(cps(reindex(term(cell), 0), var(0)))));
    break;
  case INPUT:
    retval = call(cont, cell);
    break;
  default:// WRAP, MEMOIZE, OUTPUT
    assert(0);
  };
  return retval;
}

int eval_env(int cell, int env)
{
  int retval;
  int quit = 0;
  while (!quit) {
    display(cell); fputs("\n", stderr);
    assert(is_type(cell, CALL));
    int cont = fun(cell);
    int expr = arg(cell);
    switch (type(cont)) {
    case VAR:
      cell = call(at_(env, idx(cont)), expr);
      break;
    case LAMBDA:
      cell = call(proc_stack(body(cont), env), expr);
      break;
    case CALL: {
      int cont2 = fun(cont);
      int expr2 = arg(cont);
      switch (type(cont2)) {
      case VAR:
        cell = call(call(at_(env, idx(cont2)), expr2), expr);
        break;
      case LAMBDA:
        cell = call(call(proc_stack(body(cont2), env), expr2), expr);
        break;
      case PROC:
        env = pair(wrap(expr2, env), stack(cont2));
        cell = call(term(cont2), expr);
        break;
      case WRAP:
        cell = call(call(unwrap(cont2), wrap(expr2, env)), wrap(expr, env));
        env = context(cont2);
        break;
      default:
        // VAR, LAMBDA, CALL, PROC, WRAP, MEMOIZE, INPUT, OUTPUT
        printf("fun(cont): %d ", type(cont2));
        printf("(cont: %d, ", type(cont));
        printf("expr: %d)\n", type(expr));
        assert(0);
      };
      break; }
    case PROC:
      env = pair(wrap(expr, env), stack(cont));
      cell = term(cont);
      break;
    case WRAP:
      cell = call(unwrap(cont), wrap(expr, env));
      env = context(cont);
      break;
    case OUTPUT:
      switch (type(expr)) {
      case VAR:
        cell = call(cont, at_(env, idx(expr)));
        break;
      case LAMBDA:
        cell = call(cont, proc_stack(body(expr), env));
        break;
      case PROC:
        retval = expr;
        quit = 1;
        break;
      case WRAP:
        env = context(expr);
        cell = call(cont, unwrap(expr));
        break;
      default:
        // VAR, LAMBDA, CALL, PROC, WRAP, MEMOIZE, INPUT, OUTPUT
        printf("expr: %d ", type(expr));
        printf("(cont: %d)\n", type(cont));
        assert(0);
      };
      break;
    default:
      // VAR, LAMBDA, CALL, PROC, WRAP, MEMOIZE, INPUT, OUTPUT
      printf("cont: %d ", type(cont));
      printf("(expr: %d)\n", type(expr));
      assert(0);
    };
  };
  fprintf(stderr, "-> "); display(retval); fputc('\n', stderr);
  return retval;
}

int eval(int cell)
{
  display(cell); fputc('\n', stderr);
  return eval_env(cps(cell, output()), f());
}

int is_f(int cell) { return eval(op_if(cell, t(), f())) == f(); }

int first(int list) { return call(list, t()); }
int rest(int list) { return call(list, f()); }
int empty(int list) { return call2(list, t(), proc(lambda2(f()))); }
int at(int list, int i) { return i > 0 ? at(rest(list), i - 1) : first(list); }

int op_not(int a) { return op_if(a, f(), t()); }
int op_and(int a, int b) { return op_if(a, b, f()); }
int op_or(int a, int b) { return op_if(a, t(), b); }
int op_xor(int a, int b) { return op_if(a, op_not(b), b); }
int eq_bool_ = -1;
int eq_bool(int a, int b) { return op_if(eq_bool_, a, b); }

int int_to_num(int integer)
{
  assert(integer >= 0);
  return integer == 0 ? f() : pair(integer & 0x1 ? t() : f(), int_to_num(integer >> 1));
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
  return *str == '\0' ? f() : pair(int_to_num(*str), str_to_list(str + 1));
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

const char *list_to_str_(int list) { return list_to_buffer_(list, buffer, BUFSIZE); }

const char *list_to_buffer(int list, char *buffer, int bufsize)
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

const char *list_to_str(int list) { return list_to_buffer(list, buffer, BUFSIZE); }

int id_ = -1;
int id(void) { return id_; }

int map_ = -1;
int map(int list, int fun) { return call2(map_, list, fun); }

int even_;
int even(int list) { return call(even_, list); }

int odd_;
int odd(int list) { return call(odd_, list); }

int shr_;
int shr(int list) { return call(shr_, list); }

int shl_;
int shl(int list) { return call(shl_, list); }

int zip_;
int zip(int a, int b) { return call2(zip_, a, b); }

int inject_;
int inject(int list, int start, int fun) { return call3(inject_, list, start, fun); }

int foldleft_;
int foldleft(int list, int start, int fun) { return call3(foldleft_, list, start, fun); }

int eq_num_ = -1;
int eq_num(int a, int b) { return call2(eq_num_, a, b); }

int select_if_ = -1;
int select_if(int list, int fun) { return call2(select_if_, list, fun); }

int bits_to_bytes_ = -1;
int bits_to_bytes(int bits) { return call(bits_to_bytes_, bits); }

int bytes_to_bits_ = -1;
int bytes_to_bits(int bytes) { return call(bytes_to_bits_, bytes); }

int select_binary_ = -1;
int select_binary(int list) { return call(select_binary_, list); }

void init(void)
{
  int v0 = var(0);
  int v1 = var(1);
  int v2 = var(2);
  int v3 = var(3);
  f_ = cell(PROC);
  cells[f_].proc.term = proc(v0);
  cells[f_].proc.stack = f_;
  t_ = proc(lambda(v1));
  output_ = cell(OUTPUT);
  pair_ = proc(lambda2(op_if(v0, v1, v2)));
  eq_bool_ = proc(lambda(op_if(v0, v1, op_not(v1))));
  y_ = proc(call(lambda(call(v1, call(v0, v0))),
                 lambda(call(v1, call(v0, v0)))));
  id_ = proc(v0);
  map_ = y_comb(lambda2(op_if(empty(v0),
                        f(),
                        pair(call(v1, first(v0)),
                             call2(v2, rest(v0), v1)))));
  even_ = proc(op_if(empty(v0), t(), op_not(first(v0))));
  odd_ = proc(op_if(empty(v0), f(), first(v0)));
  shr_ = proc(op_if(empty(v0), f(), rest(v0)));
  shl_ = proc(op_if(empty(v0), f(), pair(f(), v0)));
  zip_ = y_comb(lambda2(op_if(op_and(empty(v0), empty(v1)),
                              f(),
                              pair(pair(odd(v0), odd(v1)),
                                   call2(v2, shr(v0), shr(v1))))));
  inject_ = y_comb(lambda3(op_if(empty(v0),
                                 v1,
                                 call3(v3, rest(v0), call2(v2, v1, first(v0)), v2))));
  foldleft_ = y_comb(lambda3(op_if(empty(v0),
                                   v1,
                                   call2(v2, call3(v3, rest(v0), v1, v2), first(v0)))));
  eq_num_ = proc(lambda(inject(zip(v0, v1),
                               t(),
                               proc(lambda(op_and(v0, eq_bool(first(v1), rest(v1))))))));
  select_if_ = proc(lambda(foldleft(v0,
                                    f(),
                                    lambda(lambda(op_if(call(v3, v1),
                                                        pair(v1, v0),
                                                        v0))))));
  int zero = int_to_num('0');
  int one = int_to_num('1');
  bits_to_bytes_ = proc(map(v0, proc(op_if(v0, one, zero))));
  bytes_to_bits_ = proc(map(v0, proc(eq_num(v0, one))));
  select_binary_ = proc(select_if(v0, proc(op_or(eq_num(v0, zero), eq_num(v0, one)))));
}

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
  // int list = eval_env(bits_to_bytes(expr), env, output_);
  int list = eval_env(bits_to_bytes(expr), env);
  assert(0);
  while (is_f(empty(list))) {
    fputc(num_to_int(first(list)), stream);
    list = eval(rest(list));
  };
  fputc('\n', stream);
}
