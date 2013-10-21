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

#define MAX_CELLS 4000000

typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, CALLCC, CONT } type_t;

typedef struct { int fun; int arg; } call_t;

typedef struct { int block; int stack; } proc_t;

typedef struct { int unwrap; int context; int cache; } wrap_t;

typedef struct {
  type_t type;
  union {
    int idx;
    int body;
    call_t call;
    proc_t proc;
    wrap_t wrap;
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
int term(int cell) { assert(is_type(cell, CALLCC)); return cells[cell].term; }
int k(int cell) { assert(is_type(cell, CONT)); return cells[cell].k; }

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
  case CALLCC:
    retval = "callcc";
    break;
  case CONT:
    retval = "cont";
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
  if (is_type(unwrap, WRAP)) return unwrap;
  int retval = cell(WRAP);
  cells[retval].wrap.unwrap = unwrap;
  cells[retval].wrap.context = context;
  cells[retval].wrap.cache = retval;
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

int f_ = -1;
int t_ = -1;

int f(void) { return f_; }
int t(void) { return t_; }

int is_f_(int cell)
{
  return cell == f();
}

int id_ = -1;
int id(void) { return id_; }

int pair_ = -1;
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
      fputs("wrap(", stream);
      show_(unwrap(cell), stream);
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

// (+ (* 3 4) 5) -> k: (λ (v) v)              (call/cc (λ (k) (k (+ (* 3 4) 5))))
// (* 3 4)       -> k: (λ (v) (+ v 5))        (+ (call/cc (λ (k) (k (* 3 4)))) 5)
// 3             -> k: (λ (v) (+ (* v 4) 5))  (+ (* (call/cc (λ (k) (k 3))) 4) 5)
// 5             -> k: (λ (v) (+ (* 3 4) v))  (+ (* 3 4) (call/cc (λ (k) (k 5))))

// (call/cc (λ (k) (+ (* 3 (k 4)) 5)))) -> 4
// *(+ (call/cc (λ (k) k)) 5)        3) -> 8

int eval_(int cell, int env, int cc)
{
  int retval;
  int quit = 0;
  int tmp;
  while (!quit) {
    switch (type(cell)) {
    case VAR:
      // this could be a call, too!
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
      cell = unwrap(cell);
      break;
    case PROC:
      if (is_type(k(cc), VAR)) {
        retval = cell;
        quit = 1;
      } else {
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
        retval = cell;
        quit = 1;
      } else {
        tmp = cell;
        cell = arg(arg(k(cc)));
        cc = tmp;
      };
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
};

FILE *tmp_ = NULL;

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
  // procs (closures)
  assert(type(proc(lambda(var(0)), f())) == PROC);
  assert(is_type(proc(lambda(var(0)), f()), PROC));
  assert_equal(block(proc(var(0), f())), var(0));
  assert(is_f_(stack(proc(var(0), f()))));
  assert_equal(stack(proc(var(0), pair(t(), f()))), pair(t(), f()));
  // check lazy evaluation
  assert_equal(eval(call(call(t(), f()), var(123))), f());
  assert_equal(eval(call(call(f(), var(123)), f())), f());
  // identity
  assert_equal(eval(call(id(), f())), f());
  assert_equal(eval(call(id(), t())), t());
  // evaluation of calls
  assert_equal(eval(call(lambda(var(0)), f())), f());
  assert_equal(eval(call(lambda(var(0)), t())), t());
  assert_equal(eval(call(call(lambda(lambda(call(lambda(var(0)), var(1)))), f()), f())), f());
  assert_equal(eval(call(call(lambda(lambda(call(lambda(var(0)), var(1)))), t()), f())), t());
  assert_equal(eval(call(lambda(call(lambda(var(1)), f())), f())), f());
  assert_equal(eval(call(lambda(call(lambda(var(1)), f())), t())), t());
  // if
  assert_equal(eval(op_if(f(), t(), f())), f());
  assert_equal(eval(op_if(t(), t(), f())), t());
  assert_equal(eval(op_if(f(), f(), t())), t());
  assert_equal(eval(op_if(t(), f(), t())), f());
  // evaluation of lists (pairs)
  assert_equal(eval(first(pair(f(), f()))), f());
  assert_equal(eval(rest(pair(f(), f()))), f());
  assert_equal(eval(rest(pair(f(), t()))), t());
  assert_equal(eval(empty(f())), t());
  assert_equal(eval(empty(pair(f(), f()))), f());
  assert_equal(eval(at(pair(f(), pair(f(), pair(f(), f()))), 2)), f());
  assert_equal(eval(at(pair(f(), pair(f(), pair(t(), f()))), 2)), t());
  // Y-combinator
  int last = y_comb(lambda(op_if(empty(rest(var(0))), first(var(0)), call(var(1), rest(var(0))))));
  assert_equal(eval(call(last, pair(f(), f()))), f());
  assert_equal(eval(call(last, pair(t(), f()))), t());
  assert_equal(eval(call(last, pair(f(), pair(f(), f())))), f());
  assert_equal(eval(call(last, pair(f(), pair(t(), f())))), t());
  // call/cc (call with current continuation)
  assert(type(callcc(var(0))) == CALLCC);
  assert(is_type(callcc(var(0)), CALLCC));
  assert_equal(term(callcc(var(0))), var(0));
  // continuation
  assert(type(cont(var(0))) == CONT);
  assert(is_type(cont(var(0)), CONT));
  assert_equal(k(cont(var(0))), var(0));
  // evaluation of call/cc
  assert_equal(eval(callcc(f())), f());
  assert_equal(eval(callcc(call(var(0), f()))), f());
  assert_equal(eval(callcc(call(lambda(op_if(var(0), f(), t())), call(var(0), f())))), f());
  assert_equal(eval(callcc(var(0))), cont(var(0)));
  assert_equal(eval(call(call(callcc(var(0)), id()), t())), t());
  // show statistics
  fprintf(stderr, "Test suite requires %d cells.\n", cell(VAR) - n - 1);
  destroy();
  return 0;
}
