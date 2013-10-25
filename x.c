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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CELLS 4000000

typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, MEMOIZE, CALLCC, CONT, INPUT } type_t;

typedef struct { int fun; int arg; } call_t;
typedef struct { int block; int stack; } proc_t;
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
    int term;
    int k;
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
    abort();
  };
  int retval = n_cells++;
  cells[retval].type = type;
#ifndef NDEBUG
  cells[retval].tag = NULL;
#endif
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
int block(int cell) { assert(is_type(cell, PROC)); return cells[cell].proc.block; }
int stack(int cell) { assert(is_type(cell, PROC)); return cells[cell].proc.stack; }
int unwrap(int cell) { assert(is_type(cell, WRAP)); return cells[cell].wrap.unwrap; }
int context(int cell) { assert(is_type(cell, WRAP)); return cells[cell].wrap.context; }
int cache(int cell) { assert(is_type(cell, WRAP)); return cells[cell].wrap.cache; }
int value(int cell) { assert(is_type(cell, MEMOIZE)); return cells[cell].memoize.value; }
int target(int cell) { assert(is_type(cell, MEMOIZE)); return cells[cell].memoize.target; }
int term(int cell) { assert(is_type(cell, CALLCC)); return cells[cell].term; }
int k(int cell) { assert(is_type(cell, CONT)); return cells[cell].k; }
FILE *file(int cell) { assert(is_type(cell, INPUT)); return cells[cell].input.file; }
int used(int cell) { assert(is_type(cell, INPUT)); return cells[cell].input.used; }

const char *type_id(int cell)
{
  const char *retval;
  switch (type(cell)) {
  case VAR:
    retval = "var";
    break;
  case LAMBDA:
    retval = "lambda";
    break;
  case CALL:
    retval = "call";
    break;
  case PROC:
    retval = "proc";
    break;
  case WRAP:
    retval = "wrap";
    break;
  case MEMOIZE:
    retval = "memoize";
    break;
  case CALLCC:
    retval = "callcc";
    break;
  case CONT:
    retval = "cont";
    break;
  case INPUT:
    retval = "input()";
    break;
  default:
    assert(0);
  };
  return retval;
}

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
int op_if(int condition, int consequent, int alternative)
{
  return call2(condition, alternative, consequent);
}

int proc(int block, int stack)
{
  int retval = cell(PROC);
  cells[retval].proc.block = block;
  cells[retval].proc.stack = stack;
  return retval;
}

int proc_self(int block)
{
  int retval = cell(PROC);
  cells[retval].proc.block = block;
  cells[retval].proc.stack = block;
  return retval;
}

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

int callcc(int body)
{
  int retval = cell(CALLCC);
  cells[retval].body = body;
  return retval;
}

int cont(int k)
{
  int retval = cell(CONT);
  cells[retval].k = k;
  return retval;
}

int input(FILE *file)
{
  int retval = cell(INPUT);
  cells[retval].input.file = file;
  cells[retval].input.used = retval;
  return retval;
}

int f_ = -1;
int t_ = -1;
int f(void) { return f_; }
int t(void) { return t_; }

int is_f_(int cell)
{
  return cell == f();
}

int id_ = -1;
int pair_ = -1;
int id(void) { return id_; }
int pair(int first, int rest) { return call2(pair_, first, rest); }
int first_(int list) { return arg(list); }
int rest_(int list) { return arg(fun(list)); }
int at_(int list, int i)
{
  if (is_f_(list)) {
    fputs("Array out of range!\n", stderr);
    abort();
  };
  return i > 0 ? at_(rest_(list), i - 1) : first_(list);
}

int first(int list) { return call(list, t()); }
int rest(int list) { return call(list, f()); }
int empty(int list) { return call2(list, t(), lambda3(f())); }
int at(int list, int i) { return i > 0 ? at(rest(list), i - 1) : first(list); }

int y_ = -1;
int y_comb(int fun) { return call(y_, lambda(fun)); }

int eq_bool_ = -1;
int op_not(int a) { return op_if(a, f(), t()); }
int op_and(int a, int b) { return op_if(a, b, f()); }
int op_or(int a, int b) { return op_if(a, t(), b); }
int op_xor(int a, int b) { return op_if(a, op_not(b), b); }
int eq_bool(int a, int b) { return op_if(eq_bool_, a, b); }

#ifndef NDEBUG
void show_(int cell, FILE *stream)
{
  if (cells[cell].tag)
    fprintf(stream, "%s", cells[cell].tag);
  else {
    switch (type(cell)) {
    case VAR:
      fprintf(stream, "var(%d)", idx(cell));
      break;
    case LAMBDA:
      fputs("lambda(", stream);
      show_(body(cell), stream);
      fputs(")", stream);
      break;
    case CALL:
      fputs("call(", stream);
      show_(fun(cell), stream);
      fputs(", ", stream);
      show_(arg(cell), stream);
      fputs(")", stream);
      break;
    case PROC:
      fputs("proc(", stream);
      show_(block(cell), stream);
      fputs(")", stream);
      break;
    case WRAP:
      show_(unwrap(cell), stream);
      break;
    case MEMOIZE:
      fputs("memoize(", stream);
      show_(target(cell), stream);
      fputs(")", stream);
      break;
    case CALLCC:
      fputs("callcc(", stream);
      show_(term(cell), stream);
      fputs(")", stream);
      break;
    case CONT:
      fputs("cont(", stream);
      show_(k(cell), stream);
      fputs(")", stream);
      break;
    default:
      assert(0);
    };
  };
}

void show(int cell, FILE *stream)
{
  show_(cell, stream); fputc('\n', stream);
}
#endif

int int_to_num(int integer);

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

int eval_(int cell, int env, int cc)
{
  int retval;
  int quit = 0;
  int tmp;
  while (!quit) {
    switch (type(cell)) {
    case VAR:
      // this could be a call, too!
      // use continuation?
      // cell = eval_(fun(cell), env);
      cell = at_(env, idx(cell));
      break;
    case LAMBDA:
      cell = proc(body(cell), env);
      break;
    case CALL:
      cc = cont(call(cc, call(var(0), wrap(arg(cell), env))));
      cell = fun(cell);
      break;
    case WRAP:
      env = context(cell);
      if (cache(cell) != cell)
        cell = cache(cell);
      else {
        cc = cont(call(cc, memoize(var(0), cell)));
        cell = unwrap(cell);
      };
      break;
    case PROC:
      if (is_type(k(cc), VAR)) {
        assert(idx(k(cc)) == 0);
        retval = cell;
        quit = 1;
      } else if (is_type(arg(k(cc)), MEMOIZE)) {
        store(target(arg(k(cc))), cell);
        cc = fun(k(cc));
      } else {
        assert(idx(fun(arg(k(cc)))) == 0);
        env = pair(arg(arg(k(cc))), stack(cell));
        cell = block(cell);
        cc = fun(k(cc));
      };
      break;
    case CALLCC:
      env = pair(cc, env);
      cell = term(cell);
      break;
    case CONT:
      if (is_type(k(cc), VAR)) {
        assert(idx(k(cc)) == 0);
        retval = cell;
        quit = 1;
      } else {
        assert(idx(fun(arg(k(cc)))) == 0);
        tmp = cell;
        cell = arg(arg(k(cc)));
        cc = tmp;
      };
      break;
    case INPUT:
      if (is_type(k(cc), VAR)) {
        assert(idx(k(cc)) == 0);
        retval = cell;
        quit = 1;
      } else
        cell = read_char(cell);
      break;
    default:
      fprintf(stderr, "Unexpected expression type '%s' in function 'eval_'!\n", type_id(cell));
      abort();
    };
  };
  return retval;
}

int eval(int cell)
{
  return eval_(cell, f(), cont(var(0)));
}

int is_f(int cell)
{
  return eval(op_if(cell, t(), f())) == f();
}

int int_to_num(int integer)
{
  assert(integer >= 0);
  return integer == 0 ? f() : pair(integer & 0x1 ? t() : f(), int_to_num(integer >> 1));
}

int num_to_int_(int number)
{
  return is_f_(number) ? 0 : (is_f_(first_(number)) ? 0 : 0x1) | num_to_int_(rest_(number)) << 1;
}

int num_to_int(int number)
{
  int eval_num = eval(number);
  return !is_f(empty(eval_num)) ? 0 : (is_f(first(eval_num)) ? 0 : 0x1) | num_to_int(rest(eval_num)) << 1;
}

int even_;
int even(int list) { return call(even_, list); }

int odd_;
int odd(int list) { return call(odd_, list); }

int shr_;
int shr(int list) { return call(shr_, list); }

int shl_;
int shl(int list) { return call(shl_, list); }

FILE *tmp_ = NULL;

int str_to_input(const char *text)
{
  if (tmp_) fclose(tmp_);
  tmp_ = tmpfile();
  fputs(text, tmp_);
  rewind(tmp_);
  return input(tmp_);
}

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
    abort();
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
    abort();
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

void output(int expr, FILE *stream)
{
  int list = eval(expr);
  while (is_f(empty(list))) {
    fputc(num_to_int(first(list)), stream);
    list = eval(rest(list));
  };
}

int eq(int a, int b)
{
  int retval;
  if (a == b)
    retval = 1;
  else if (type(a) == type(b)) {
    switch (type(a)) {
    case VAR:
      retval = idx(a) == idx(b);
      break;
    case LAMBDA:
      retval = eq(body(a), body(b));
      break;
    case CALL:
      retval = eq(fun(a), fun(b)) && eq(arg(a), arg(b));
      break;
    case PROC:
      retval = eq(block(a), block(b)) && eq(stack(a), stack(b));
      break;
    case WRAP:
      retval = eq(unwrap(a), unwrap(b)) && eq(context(a), context(b));
      break;
    case MEMOIZE:
      retval = eq(value(a), value(b)) && eq(target(a), target(b));
      break;
    case CALLCC:
      retval = eq(term(a), term(b));
      break;
    case CONT:
      retval = eq(k(a), k(b));
      break;
    default:
      assert(0);
    }
  } else
    retval = 0;
  return retval;
}

#define assert_equal(a, b) \
  ((void) (eq(a, b) ? 0 : __assert_equal(#a, #b, __FILE__, __LINE__)))
#define __assert_equal(a, b, file, line) \
  ((void) printf("%s:%u: failed assertion `%s' not equal to `%s'\n", file, line, a, b), abort())

void init(void)
{
  int v0 = var(0);
  int v1 = var(1);
  int v2 = var(2);
  f_ = proc_self(lambda(v0));
  t_ = proc(lambda(v1), f());
  id_ = proc(v0, f());
  pair_ = lambda3(op_if(v0, v1, v2));
  y_ = lambda(call(lambda(call(v1, call(v0, v0))), lambda(call(v1, call(v0, v0)))));
  eq_bool_ = lambda2(op_if(v0, v1, op_not(v1)));
  even_ = lambda(op_if(empty(v0), t(), op_not(first(v0))));
  odd_ = lambda(op_if(empty(v0), f(), first(v0)));
  shr_ = lambda(op_if(empty(v0), f(), rest(v0)));
  shl_ = lambda(op_if(empty(v0), f(), pair(f(), v0)));
};

void destroy(void)
{
  if (tmp_) fclose(tmp_);
  tmp_ = NULL;
}

int main(void)
{
  init();
  int n = cell(VAR);
  // variable
  assert(type(var(0)) == VAR);
  assert(is_type(var(0), VAR));
  assert(idx(var(1)) == 1);
  // lambda (function)
  assert(type(lambda(var(0))) == LAMBDA);
  assert(is_type(lambda(var(0)), LAMBDA));
  assert_equal(body(lambda(var(0))), var(0));
  assert_equal(lambda2(var(0)), lambda(lambda(var(0))));
  assert_equal(lambda3(var(0)), lambda(lambda(lambda(var(0)))));
  // call
  assert(type(call(lambda(var(0)), var(0))) == CALL);
  assert(is_type(call(lambda(var(0)), var(0)), CALL));
  assert_equal(fun(call(lambda(var(1)), var(2))), lambda(var(1)));
  assert_equal(arg(call(lambda(var(1)), var(2))), var(2));
  assert_equal(call2(var(0), var(1), var(2)), call(call(var(0), var(2)), var(1)));
  assert_equal(call3(var(0), var(1), var(2), var(3)), call(call(call(var(0), var(3)), var(2)), var(1)));
  // booleans
  assert(is_f_(f()));
  assert(!is_f_(t()));
  assert_equal(eval(f()), f());
  assert_equal(eval(t()), t());
  assert(is_f(f()));
  assert(!is_f(t()));
  // conditional
  assert_equal(op_if(var(1), var(2), var(3)), call(call(var(1), var(2)), var(3)));
  // lists (pairs)
  assert(!is_f_(pair(t(), f())));
  assert_equal(first_(pair(var(1), f())), var(1));
  assert(is_f_(rest_(pair(var(1), f()))));
  assert_equal(at_(pair(var(1), pair(var(2), pair(var(3), f()))), 0), var(1));
  assert_equal(at_(pair(var(1), pair(var(2), pair(var(3), f()))), 1), var(2));
  assert_equal(at_(pair(var(1), pair(var(2), pair(var(3), f()))), 2), var(3));
  // wraps
  assert(type(wrap(var(0), pair(f(), f()))) == WRAP);
  assert(is_type(wrap(var(0), pair(f(), f())), WRAP));
  assert_equal(unwrap(wrap(var(0), pair(f(), f()))), var(0));
  assert_equal(context(wrap(var(0), pair(f(), f()))), pair(f(), f()));
  int w = wrap(var(0), f()); assert(cache(w) == w);
  store(w, f()); assert(cache(w) == f());
  // memoization
  assert(type(memoize(var(0), wrap(f(), f()))) == MEMOIZE);
  assert(is_type(memoize(var(0), wrap(f(), f())), MEMOIZE));
  assert_equal(value(memoize(var(0), wrap(f(), f()))), var(0));
  assert_equal(target(memoize(var(0), wrap(f(), f()))), wrap(f(), f()));
  // memoization of values
  int duplicate = eval(call(lambda(pair(var(0), var(0))), f()));
  assert(cache(first_(stack(duplicate))) == first_(stack(duplicate)));
  assert(is_f(eval(first(duplicate))));
  assert(cache(first_(stack(duplicate))) == f());
  store(first_(stack(duplicate)), t());
  assert(!is_f(eval(first(duplicate))));
  // procs (closures)
  assert(type(proc(lambda(var(0)), f())) == PROC);
  assert(is_type(proc(lambda(var(0)), f()), PROC));
  assert_equal(block(proc(var(0), f())), var(0));
  assert(is_f_(stack(proc(var(0), f()))));
  assert_equal(stack(proc(var(0), pair(t(), f()))), pair(t(), f()));
  // check lazy evaluation
  assert(is_f(call(call(t(), f()), var(123))));
  assert(is_f(call(call(f(), var(123)), f())));
  // Evaluation of variables and functions
  assert(is_f(wrap(var(0), pair(f(), f()))));
  assert(!is_f(wrap(var(0), pair(t(), f()))));
  assert_equal(eval(lambda(var(0))), proc(var(0), f()));
  // identity
  assert(is_f(call(id(), f())));
  assert(!is_f(call(id(), t())));
  // evaluation of calls
  assert(is_f(call(lambda(var(0)), f())));
  assert(!is_f(call(lambda(var(0)), t())));
  assert(is_f(call(call(lambda(lambda(call(lambda(var(0)), var(1)))), f()), f())));
  assert(!is_f(call(call(lambda(lambda(call(lambda(var(0)), var(1)))), t()), f())));
  assert(is_f(call(lambda(call(lambda(var(1)), f())), f())));
  assert(!is_f(call(lambda(call(lambda(var(1)), f())), t())));
  // if-statement
  assert(is_f(op_if(f(), t(), f())));
  assert(!is_f(op_if(t(), t(), f())));
  assert(!is_f(op_if(f(), f(), t())));
  assert(is_f(op_if(t(), f(), t())));
  // evaluation of lists (pairs)
  assert(is_f(first(pair(f(), f()))));
  assert(is_f(rest(pair(f(), f()))));
  assert(!is_f(rest(pair(f(), t()))));
  assert(!is_f(empty(f())));
  assert(is_f(empty(pair(f(), f()))));
  assert(is_f(at(pair(f(), pair(f(), pair(f(), f()))), 2)));
  assert(!is_f(at(pair(f(), pair(f(), pair(t(), f()))), 2)));
  // Y-combinator
  int last = y_comb(lambda(op_if(empty(rest(var(0))), first(var(0)), call(var(1), rest(var(0))))));
  assert(is_f(call(last, pair(f(), f()))));
  assert(!is_f(call(last, pair(t(), f()))));
  assert(is_f(call(last, pair(f(), pair(f(), f())))));
  assert(!is_f(call(last, pair(f(), pair(t(), f())))));
  // call/cc (call with current continuation)
  assert(type(callcc(var(0))) == CALLCC);
  assert(is_type(callcc(var(0)), CALLCC));
  assert_equal(term(callcc(var(0))), var(0));
  // continuation
  assert(type(cont(var(0))) == CONT);
  assert(is_type(cont(var(0)), CONT));
  assert_equal(k(cont(var(0))), var(0));
  // evaluation of call/cc
  assert(is_f(callcc(f())));
  assert(is_f(callcc(call(var(0), f()))));
  assert(is_f(callcc(call(lambda(op_if(var(0), f(), t())), call(var(0), f())))));
  assert_equal(eval(callcc(var(0))), cont(var(0)));
  assert(!is_f(call(call(callcc(var(0)), id()), t())));
  assert(is_f(callcc(y_comb(call(var(1), f())))));
  // boolean 'not'
  assert(!is_f(op_not(f())));
  assert(is_f(op_not(t())));
  // boolean 'and'
  assert(is_f(op_and(f(), f())));
  assert(is_f(op_and(f(), t())));
  assert(is_f(op_and(t(), f())));
  assert(!is_f(op_and(t(), t())));
  // boolean 'or'
  assert(is_f(op_or(f(), f())));
  assert(!is_f(op_or(f(), t())));
  assert(!is_f(op_or(t(), f())));
  assert(!is_f(op_or(t(), t())));
  // boolean 'xor'
  assert(is_f(op_xor(f(), f())));
  assert(!is_f(op_xor(f(), t())));
  assert(!is_f(op_xor(t(), f())));
  assert(is_f(op_xor(t(), t())));
  // boolean '=='
  assert(!is_f(eq_bool(f(), f())));
  assert(is_f(eq_bool(f(), t())));
  assert(is_f(eq_bool(t(), f())));
  assert(!is_f(eq_bool(t(), t())));
  // numbers
  assert(is_f_(int_to_num(0)));
  assert(!is_f_(at_(int_to_num(1), 0)));
  assert(is_f_(at_(int_to_num(2), 0)));
  assert(!is_f_(at_(int_to_num(2), 1)));
  assert(num_to_int_(int_to_num(123)) == 123);
  assert(num_to_int(first(pair(int_to_num(123), f()))) == 123);
  // even and odd numbers
  assert(is_f(even(int_to_num(77))));
  assert(!is_f(even(int_to_num(50))));
  assert(!is_f(odd(int_to_num(77))));
  assert(is_f(odd(int_to_num(50))));
  // shift -left and shift-right
  assert(num_to_int(shl(int_to_num(77))) == 154);
  assert(num_to_int(shr(int_to_num(77))) == 38);
  // input
  assert(type(input(stdin)) == INPUT);
  assert(is_type(input(stdin), INPUT));
  assert(file(input(stdin)) == stdin);
  assert(file(used(input(stdin))) == stdin);
  // convert string to input object
  int in1 = str_to_input("abc");
  assert(fgetc(file(in1)) == 'a');
  assert(fgetc(file(in1)) == 'b');
  assert(fgetc(file(in1)) == 'c');
  // read characters from input object
  int in2 = str_to_input("ab");
  assert(num_to_int_(first_(read_char(in2))) == 'a');
  assert(num_to_int_(first_(read_char(rest_(read_char(in2))))) == 'b');
  assert(is_f_(read_char(rest_(read_char(rest_(read_char(in2)))))));
  // evaluation of input
  int in3 = str_to_input("abc");
  assert(num_to_int(first(in3)) == 'a');
  assert(num_to_int(first(rest(rest(in3)))) == 'c');
  assert(num_to_int(first(rest(in3))) == 'b');
  assert(is_f(rest(rest(rest(in3)))));
  // convert string to list
  int list = str_to_list("abc");
  assert(num_to_int_(first_(list)) == 'a');
  assert(num_to_int_(first_(rest_(rest_(list)))) == 'c');
  assert(num_to_int_(first_(rest_(list))) == 'b');
  // convert list to string
  assert(!strcmp(list_to_str_(str_to_list("abc")), "abc"));
  // convert expression to string
  assert(!strcmp(list_to_str(call(lambda(pair(var(0), pair(var(0), f()))), int_to_num('x'))), "xx"));
  // write expression to stream
  FILE *of = tmpfile();
  output(str_to_input("xy"), of);
  rewind(of);
  assert(fgetc(of) == 'x');
  assert(fgetc(of) == 'y');
  assert(fgetc(of) == EOF);
  fclose(of);
  // show statistics
  fprintf(stderr, "Test suite requires %d cells.\n", cell(VAR) - n - 1);
  destroy();
  return 0;
}
