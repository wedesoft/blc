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

#define MAX_CELLS 8000000

typedef enum { VAR,
               LAMBDA,
               CALL,
               PROC,
               WRAP,
               MEMOIZE,
               CALLCC,
               CONT,
               ISTREAM,
               STRING,
               INTEGER } type_t;

typedef struct { int fun; int arg; } call_t;
typedef struct { int block; int stack; } proc_t;
typedef struct { int unwrap; int context; int cache; } wrap_t;
typedef struct { int value; int target; } memoize_t;
typedef struct { FILE *file; int used; } istream_t;

typedef struct {
  type_t type;
  union {
    int idx;
    int body;
    call_t call;
    proc_t proc;
    wrap_t wrap;
    memoize_t memoize;
    istream_t istream;
    const char *string;
    int term;
    int k;
    int integer;
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
FILE *file(int cell) { assert(is_type(cell, ISTREAM)); return cells[cell].istream.file; }
int used(int cell) { assert(is_type(cell, ISTREAM)); return cells[cell].istream.used; }
const char *string(int cell) { assert(is_type(cell, STRING)); return cells[cell].string; }
int intval(int cell) { assert(is_type(cell, INTEGER)); return cells[cell].integer; }

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
  case ISTREAM:
    retval = "istream";
    break;
  case STRING:
    retval = "string";
    break;
  case INTEGER:
    retval = "integer";
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

int from_file(FILE *file)
{
  int retval = cell(ISTREAM);
  cells[retval].istream.file = file;
  cells[retval].istream.used = retval;
  return retval;
}

int from_str(const char *string)
{
  int retval = cell(STRING);
  cells[retval].string = string;
  return retval;
}

int from_int(int integer)
{
  int retval = cell(INTEGER);
  cells[retval].integer = integer;
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
int list1(int a) { return pair(a, f()); }
int list2(int a, int b) { return pair(a, list1(b)); }
int list3(int a, int b, int c) { return pair(a, list2(b, c)); }
int list4(int a, int b, int c, int d) { return pair(a, list3(b, c, d)); }
int list5(int a, int b, int c, int d, int e) { return pair(a, list4(b, c, d, e)); }
int list6(int a, int b, int c, int d, int e, int f) { return pair(a, list5(b, c, d, e, f)); }

int first(int list) { return call(list, t()); }
int rest(int list) { return call(list, f()); }
int empty(int list) { return call2(list, t(), lambda3(f())); }
int at(int list, int i) { return i > 0 ? at(rest(list), i - 1) : first(list); }

// Y-combinator
int recursive_ = -1;
int recursive(int fun) { return call(recursive_, lambda(fun)); }

int eq_bool_ = -1;
int op_not(int a) { return op_if(a, f(), t()); }
int op_and(int a, int b) { return op_if(a, b, f()); }
int op_or(int a, int b) { return op_if(a, t(), b); }
int op_xor(int a, int b) { return op_if(a, op_not(b), b); }
int eq_bool(int a, int b) { return call2(eq_bool_, a, b); }

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

int read_stream(int in)
{
  int retval;
  if (used(in) != in)
    retval = used(in);
  else {
    int c = fgetc(file(in));
    if (c == EOF)
      retval = f();
    else
      retval = pair(from_int(c), from_file(file(in)));
    cells[in].istream.used = retval;
  }
  return retval;
}

int read_string(int str)
{
  char c = *string(str);
  return c == '\0' ? f() : pair(from_int(c), from_str(string(str) + 1));
}

int read_integer(int cell)
{
  int value = intval(cell);
  assert(value >= 0);
  return value == 0 ? f() : pair(value & 0x1 ? t() : f(), from_int(value >> 1));
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
    case ISTREAM:
      if (is_type(k(cc), VAR)) {
        assert(idx(k(cc)) == 0);
        retval = cell;
        quit = 1;
      } else
        cell = read_stream(cell);
      break;
    case STRING:
      if (is_type(k(cc), VAR)) {
        assert(idx(k(cc)) == 0);
        retval = cell;
        quit = 1;
      } else
        cell = read_string(cell);
      break;
    case INTEGER:
      if (is_type(k(cc), VAR)) {
        assert(idx(k(cc)) == 0);
        retval = cell;
        quit = 1;
      } else
        cell = read_integer(cell);
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

int to_int(int number)
{
  int eval_num = eval(number);
  return !is_f(empty(eval_num)) ? 0 : (is_f(first(eval_num)) ? 0 : 0x1) | to_int(rest(eval_num)) << 1;
}

int even_;
int even(int list) { return call(even_, list); }

int odd_;
int odd(int list) { return call(odd_, list); }

int shr_;
int shr(int list) { return call(shr_, list); }

int shl_;
int shl(int list) { return call(shl_, list); }

#define BUFSIZE 1024
char buffer[BUFSIZE];

const char *to_buffer(int list, char *buffer, int bufsize)
{
  if (bufsize <= 1) {
    fputs("Buffer too small!\n", stderr);
    abort();
  };
  int eval_list = eval(list);
  if (!is_f(empty(eval_list)))
    *buffer = '\0';
  else {
    *buffer = to_int(first(eval_list));
    to_buffer(rest(list), buffer + 1, bufsize - 1);
  };
  return buffer;
}

const char *to_str(int list) { return to_buffer(list, buffer, BUFSIZE); }

int eq_list_ = -1;
int eq_list(int eq_elem) { return call(eq_list_, eq_elem); }
int eq_num_ = -1;
int eq_num(int a, int b) { return call2(eq_num_, a, b); }
int eq_str_ = -1;
int eq_str(int a, int b) { return call2(eq_str_, a, b); }

int lookup_ = -1;
int lookup(int alist, int eq_elem, int other)
{
  return call3(lookup_, alist, eq_elem, other);
}
int lookup_bool(int alist, int other)
{
  return lookup(alist, eq_bool_, lambda(other));
}
int lookup_num(int alist, int other)
{
  return lookup(alist, eq_num_, lambda(other));
}
int lookup_str(int alist, int other)
{
  return lookup(alist, eq_str_, lambda(other));
}

void output(int expr, FILE *stream)
{
  int list = eval(expr);
  while (is_f(empty(list))) {
    fputc(to_int(first(list)), stream);
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

void init(void)
{
  int v0 = var(0);
  int v1 = var(1);
  int v2 = var(2);
  f_ = proc_self(lambda(v0));
  t_ = proc(lambda(v1), f());
  id_ = proc(v0, f());
  pair_ = lambda3(op_if(v0, v1, v2));
  recursive_ = lambda(call(lambda(call(v1, call(v0, v0))),
                           lambda(call(v1, call(v0, v0)))));
  eq_bool_ = lambda2(op_if(v0, v1, op_not(v1)));
  even_ = lambda(op_if(empty(v0), t(), op_not(first(v0))));
  odd_ = lambda(op_if(empty(v0), f(), first(v0)));
  shr_ = lambda(op_if(empty(v0), f(), rest(v0)));
  shl_ = lambda(op_if(empty(v0), f(), pair(f(), v0)));
  eq_list_ = lambda(recursive(lambda2(op_if(op_and(empty(var(0)), empty(var(1))),
                    t(),
                    op_if(op_or(empty(var(0)), empty(var(1))),
                          f(),
                          op_and(call2(var(3), first(var(0)), first(var(1))),
                                 call2(var(2), rest(var(0)), rest(var(1)))))))));
  eq_num_ = eq_list(eq_bool_);
  eq_str_ = eq_list(eq_num_);
  lookup_ = lambda2(recursive(lambda2(op_if(empty(var(1)),
                    call(var(4), var(0)),
                    op_if(call2(var(3), first(first(var(1))), var(0)),
                          rest(first(var(1))),
                          call2(var(2), var(0), rest(var(1))))))));
};

void destroy(void)
{
}

#define assert_equal(a, b) \
  ((void) (eq(a, b) ? 0 : __assert_equal(#a, #b, __FILE__, __LINE__)))
#define __assert_equal(a, b, file, line) \
  ((void) printf("%s:%u: failed assertion `%s' not equal to `%s'\n", file, line, a, b), abort())

int methods(int alist, int other)
{
  return lookup(alist, eq_str_, other);
}

int send(int self, const char *msg)
{
  return call(call(self, from_str(msg)), self);
}

int send1(int self, const char *msg, int arg)
{
  return call2(call(self, from_str(msg)), self, arg);
}

int superclass(int self)
{
  return send(self, "superclass");
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
  assert(!is_f_(list1(t())));
  assert_equal(first_(list1(var(1))), var(1));
  assert(is_f_(rest_(list1(var(1)))));
  assert_equal(at_(list3(var(1), var(2), var(3)), 0), var(1));
  assert_equal(at_(list3(var(1), var(2), var(3)), 1), var(2));
  assert_equal(at_(list3(var(1), var(2), var(3)), 2), var(3));
  // wraps
  assert(type(wrap(var(0), list1(f()))) == WRAP);
  assert(is_type(wrap(var(0), list1(f())), WRAP));
  assert_equal(unwrap(wrap(var(0), list1(f()))), var(0));
  assert_equal(context(wrap(var(0), list1(f()))), list1(f()));
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
  assert_equal(stack(proc(var(0), list1(t()))), list1(t()));
  // check lazy evaluation
  assert(is_f(call(call(t(), f()), var(123))));
  assert(is_f(call(call(f(), var(123)), f())));
  // Evaluation of variables and functions
  assert(is_f(wrap(var(0), list1(f()))));
  assert(!is_f(wrap(var(0), list1(t()))));
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
  assert(is_f(first(list1(f()))));
  assert(is_f(rest(list1(f()))));
  assert(!is_f(rest(pair(f(), t()))));
  assert(!is_f(empty(f())));
  assert(is_f(empty(list1(f()))));
  assert(is_f(at(list3(f(), f(), f()), 2)));
  assert(!is_f(at(list3(f(), f(), t()), 2)));
  // Y-combinator
  int last = recursive(lambda(op_if(empty(rest(var(0))), first(var(0)), call(var(1), rest(var(0))))));
  assert(is_f(call(last, list1(f()))));
  assert(!is_f(call(last, list1(t()))));
  assert(is_f(call(last, list2(f(), f()))));
  assert(!is_f(call(last, list2(f(), t()))));
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
  assert(is_f(callcc(recursive(call(var(1), f())))));
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
  int x = from_int(2);
  assert(is_f_(first_(read_integer(x))));
  assert(intval(rest_(read_integer(x))) == 1);
  assert(!is_f(first_(read_integer(rest_(read_integer(x))))));
  assert(is_f(read_integer(rest_(read_integer(rest_(read_integer(x)))))));
  assert(is_f(from_int(0)));
  assert(!is_f(at(from_int(1), 0)));
  assert(is_f(at(from_int(2), 0)));
  assert(!is_f(at(from_int(2), 1)));
  assert(to_int(from_int(123)) == 123);
  assert(to_int(first(list1(from_int(123)))) == 123);
  // even and odd numbers
  assert(is_f(even(from_int(77))));
  assert(!is_f(even(from_int(50))));
  assert(!is_f(odd(from_int(77))));
  assert(is_f(odd(from_int(50))));
  // shift -left and shift-right
  assert(to_int(shl(from_int(77))) == 154);
  assert(to_int(shr(from_int(77))) == 38);
  // strings
  int str = from_str("ab");
  assert(to_int(first_(read_string(str))) == 'a');
  assert(to_int(first_(read_string(rest_(read_string(str))))) == 'b');
  assert(is_f(read_string(rest_(read_string(rest_(read_string(str)))))));
  // evaluation of string expressions
  assert(!strcmp(to_str(from_str("abc")), "abc"));
  assert(!strcmp(to_str(call(lambda(list2(var(0), var(0))), from_int('x'))), "xx"));
  // list equality
  assert(is_f(eq_num(from_int(0), from_int(1))));
  assert(is_f(eq_num(from_int(1), from_int(0))));
  assert(is_f(eq_num(from_int(1), from_int(2))));
  assert(is_f(eq_num(from_int(2), from_int(1))));
  assert(!is_f(eq_num(from_int(0), from_int(0))));
  assert(!is_f(eq_num(from_int(1), from_int(1))));
  assert(!is_f(eq_num(from_int(2), from_int(2))));
  // string equality
  assert(is_f(eq_str(from_str("abc"), from_str("apc"))));
  assert(is_f(eq_str(from_str("ab"), from_str("abc"))));
  assert(is_f(eq_str(from_str("abc"), from_str("ab"))));
  assert(!is_f(eq_str(from_str("abc"), from_str("abc"))));
  // association list with booleans
  int alist1 = lookup_bool(list2(pair(t(), from_int(1)),
                                 pair(f(), from_int(0))),
                           f());
  assert(to_int(call(alist1, f())) == 0);
  assert(to_int(call(alist1, t())) == 1);
  // association list with numbers
  int alist2 = lookup_num(list3(pair(from_int(2), from_int(1)),
                                pair(from_int(3), from_int(2)),
                                pair(from_int(5), from_int(3))),
                          from_int(0));
  assert(to_int(call(alist2, from_int(2))) == 1);
  assert(to_int(call(alist2, from_int(3))) == 2);
  assert(to_int(call(alist2, from_int(5))) == 3);
  assert(to_int(call(alist2, from_int(4))) == 0);
  // association list with strings
  int alist3 = lookup_str(list2(pair(from_str("Jan"), from_int(31)),
                                pair(from_str("Feb"), from_int(28))),
                          from_int(30));
  assert(to_int(call(alist3, from_str("Jan"))) == 31);
  assert(to_int(call(alist3, from_str("Feb"))) == 28);
  assert(to_int(call(alist3, from_str("Mar"))) == 30);
  // input file stream
  assert(type(from_file(stdin)) == ISTREAM);
  assert(is_type(from_file(stdin), ISTREAM));
  assert(file(from_file(stdin)) == stdin);
  assert(file(used(from_file(stdin))) == stdin);
  int in1 = from_file(tmpfile());
  fputs("ab", file(in1));
  rewind(file(in1));
  assert(to_int(first_(read_stream(in1))) == 'a');
  assert(to_int(first_(read_stream(rest_(read_stream(in1))))) == 'b');
  assert(is_f(read_stream(rest_(read_stream(rest_(read_stream(in1)))))));
  fclose(file(in1));
  // integers
  assert(type(from_int(5)) == INTEGER);
  assert(is_type(from_int(5), INTEGER));
  assert(intval(from_int(5)) == 5);
  // evaluation of input
  int in3 = from_str("abc");
  assert(to_int(first(in3)) == 'a');
  assert(to_int(first(rest(rest(in3)))) == 'c');
  assert(to_int(first(rest(in3))) == 'b');
  assert(is_f(rest(rest(rest(in3)))));
  // write expression to stream
  FILE *of = tmpfile();
  output(from_str("xy"), of);
  rewind(of);
  assert(fgetc(of) == 'x');
  assert(fgetc(of) == 'y');
  assert(fgetc(of) == EOF);
  fclose(of);
  // classes
  int oc_ = lambda(recursive(methods(
    list2(pair(from_str("inspect"), lambda(from_str("Object"))),
          pair(from_str("to_s"), lambda(send(var(0), "inspect")))),
    lambda(f()))));
  int fc_ = lambda(recursive(methods(
    list5(pair(from_str("superclass"), lambda(call(var(2), from_str("Object")))),
          pair(from_str("inspect"), lambda(from_str("false"))),
          pair(from_str("not"), lambda(call(var(2), from_str("true")))),
          pair(from_str("and"), lambda2(var(0))),
          pair(from_str("or"), lambda2(var(1)))),
    lambda(call(superclass(var(1)), var(0))))));
  int tc_ = lambda(recursive(methods(
    list5(pair(from_str("superclass"), lambda(call(var(2), from_str("Object")))),
          pair(from_str("inspect"), lambda(from_str("true"))),
          pair(from_str("not"), lambda(call(var(2), from_str("false")))),
          pair(from_str("and"), lambda2(var(1))),
          pair(from_str("or"), lambda2(var(0)))),
    lambda(call(superclass(var(1)), var(0))))));
  int env = recursive(lookup_str(
    list3(pair(from_str("Object"), call(oc_, var(0))),
          pair(from_str("false"), call(fc_, var(0))),
          pair(from_str("true"), call(tc_, var(0)))),
    f()));
  int oc = call(oc_, env);
  int fc = call(fc_, env);
  int tc = call(tc_, env);
  assert(!strcmp(to_str(send(oc, "inspect")), "Object"));
  assert(!strcmp(to_str(send(oc, "to_s")), "Object"));
  assert(!strcmp(to_str(send(fc, "inspect")), "false"));
  assert(!strcmp(to_str(send(fc, "to_s")), "false"));
  assert(!strcmp(to_str(send(tc, "inspect")), "true"));
  assert(!strcmp(to_str(send(tc, "to_s")), "true"));
  assert(!strcmp(to_str(send(send(fc, "superclass"), "inspect")), "Object"));
  assert(!strcmp(to_str(send(send(tc, "superclass"), "inspect")), "Object"));
  assert(!strcmp(to_str(send(send(tc, "not"), "inspect")), "false"));
  assert(!strcmp(to_str(send(send(fc, "not"), "inspect")), "true"));
  assert(!strcmp(to_str(send(send(tc, "not"), "inspect")), "false"));
  assert(!strcmp(to_str(send(send1(fc, "and", fc), "inspect")), "false"));
  assert(!strcmp(to_str(send(send1(fc, "and", tc), "inspect")), "false"));
  assert(!strcmp(to_str(send(send1(tc, "and", fc), "inspect")), "false"));
  assert(!strcmp(to_str(send(send1(tc, "and", tc), "inspect")), "true"));
  assert(!strcmp(to_str(send(send1(fc, "or", fc), "inspect")), "false"));
  assert(!strcmp(to_str(send(send1(fc, "or", tc), "inspect")), "true"));
  assert(!strcmp(to_str(send(send1(tc, "or", fc), "inspect")), "true"));
  assert(!strcmp(to_str(send(send1(tc, "or", tc), "inspect")), "true"));
  // show statistics
  fprintf(stderr, "Test suite requires %d cells.\n", cell(VAR) - n - 1);
  destroy();
  return 0;
}
