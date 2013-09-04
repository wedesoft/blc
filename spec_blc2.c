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
#include <string.h>

#define MAX_CELLS 1048576
typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, INPUT } type_t;

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

int is_var(int cell) { return type(cell) == VAR; }
int is_lambda(int cell) { return type(cell) == LAMBDA; }
int is_call(int cell) { return type(cell) == CALL; }
int is_proc(int cell) { return type(cell) == PROC; }
int is_wrap(int cell) { return type(cell) == WRAP; }
int is_input(int cell) { return type(cell) == INPUT; }

int idx(int cell) { assert(is_var(cell)); return cells[cell].idx; }
int body(int cell) { assert(is_lambda(cell)); return cells[cell].body; }
int fun(int cell) { assert(is_call(cell)); return cells[cell].call.fun; }
int arg(int cell) { assert(is_call(cell)); return cells[cell].call.arg; }
int term(int cell) { assert(is_proc(cell)); return cells[cell].proc.term; }
int stack(int cell) { assert(is_proc(cell)); return cells[cell].proc.stack; }
int unwrap(int cell) { assert(is_wrap(cell)); return cells[cell].wrap.unwrap; }
int context(int cell) { assert(is_wrap(cell)); return cells[cell].wrap.context; }
int cache(int cell) { assert(is_wrap(cell)); return cells[cell].wrap.cache; }
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
  return is_proc(cell) && is_lambda(term(cell)) && is_var(body(term(cell))) && idx(body(term(cell))) == 0;
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

int input(FILE *file)
{
  int retval = cell(INPUT);
  cells[retval].input.file = file;
  cells[retval].input.used = retval;
  return retval;
}

int int_to_num(int integer);

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

int eval_env(int cell, int env)
{
  int retval;
  switch (type(cell)) {
    case VAR:
      retval = eval_env(at_(env, idx(cell)), env);
      break;
    case LAMBDA:
      retval = proc_stack(body(cell), env);
      break;
    case CALL: {
      int f = eval_env(fun(cell), env);
      if (is_input(f)) {
        int bit = eval_env(read_char(f), env);
        retval = eval_env(call(bit, arg(cell)), env);
      } else
        retval = eval_env(term(f), pair(wrap(arg(cell), env), stack(f)));
      break; }
    case WRAP:
      if (cache(cell) != cell)
        retval = cache(cell);
      else {
        retval = eval_env(unwrap(cell), context(cell));
        cells[cell].wrap.cache = retval;
      };
      break;
    default:
      retval = cell;
  };
  return retval;
}

int eval(int cell) { return eval_env(cell, f_); }

int is_f(int cell) { return eval(op_if(cell, t_, f_)) == f_; }

int first(int list) { return call(list, t_); }
int rest(int list) { return call(list, f_); }
int empty(int list) { return call(call(list, proc(lambda(lambda(f_)))), t_); }
int at(int list, int i) { return i > 0 ? at(rest(list), i - 1) : first(list); }

int op_not_ = -1;
int op_not(int a) { return call(op_not_, a); }
int op_and(int a, int b) { return op_if(a, b, a); }
int op_or(int a, int b) { return op_if(a, a, b); }
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
  return is_f(eval_num) ? 0 : (is_f(first(eval_num)) ? 0 : 1) | num_to_int(rest(eval_num)) << 1;
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
  if (is_f(eval_list))
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
  cells[f_].proc.term = lambda(var(0));
  cells[f_].proc.stack = f_;
  t_ = proc(lambda(var(1)));
  pair_ = proc(lambda(lambda(op_if(var(0), var(1), var(2)))));
  op_not_ = proc(lambda(lambda(op_if(var(2), var(0), var(1)))));
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
  bits_to_bytes_ = lambda(map(var(0), lambda(op_if(var(0), int_to_num('1'), int_to_num('0')))));
  bytes_to_bits_ = lambda(map(var(0), lambda(eq_num(var(0), int_to_num('1')))));
  select_binary_ = lambda(select_if(var(0), lambda(op_or(eq_num(var(0), int_to_num('0')),
                                                         eq_num(var(0), int_to_num('1'))))));
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



int main(void)
{
  init();
  // variable
  assert(type(var(0)) == VAR);
  assert(is_var(var(0)));
  assert(idx(var(1)) == 1);
  // lambda (function)
  assert(type(lambda(var(0))) == LAMBDA);
  assert(is_lambda(lambda(var(0))));
  assert(idx(body(lambda(var(1)))) == 1);
  // call
  assert(type(call(lambda(var(0)), var(0))) == CALL);
  assert(is_call(call(lambda(var(0)), var(0))));
  assert(idx(body(fun(call(lambda(var(1)), var(2))))) == 1);
  assert(idx(arg(call(lambda(var(1)), var(2)))) == 2);
  // false and true
  assert(idx(body(term(f()))) == 0);
  assert(idx(body(term(t()))) == 1);
  assert(is_f_(f()));
  assert(!is_f_(t()));
  // conditional
  assert(idx(fun(fun(op_if(var(1), var(2), var(3))))) == 1);
  assert(idx(arg(fun(op_if(var(1), var(2), var(3))))) == 2);
  assert(idx(arg(op_if(var(1), var(2), var(3)))) == 3);
  // lists (pairs)
  assert(!is_f_(pair(t(), f())));
  assert(idx(first_(pair(var(1), f()))) == 1);
  assert(is_f_(rest_(pair(var(1), f()))));
  assert(idx(at_(pair(var(1), pair(var(2), pair(var(3), f()))), 0)) == 1);
  assert(idx(at_(pair(var(1), pair(var(2), pair(var(3), f()))), 1)) == 2);
  assert(idx(at_(pair(var(1), pair(var(2), pair(var(3), f()))), 2)) == 3);
  // procs (closures)
  assert(type(proc(lambda(var(0)))) == PROC);
  assert(is_proc(proc(lambda(var(0)))));
  assert(idx(term(proc(var(0)))) == 0);
  assert(is_f_(stack(proc_stack(var(0), f()))));
  // evaluation of lambdas
  assert(is_proc(eval(lambda(var(0)))));
  assert(idx(term(eval(lambda(var(1))))) == 1);
  assert(is_f_(stack(eval(lambda(var(0))))));
  // wraps
  assert(type(wrap(var(0), pair(f(), f()))) == WRAP);
  assert(is_wrap(wrap(var(0), pair(f(), f()))));
  assert(idx(unwrap(wrap(var(0), pair(f(), f())))) == 0);
  assert(is_f_(context(wrap(var(0), f()))));
  // evaluation of variables
  assert(is_f(wrap(var(1), pair(f(), pair(f(), f())))));
  assert(!is_f(wrap(var(1), pair(f(), pair(t(), f())))));
  // evaluation of calls
  assert(is_f(call(lambda(var(0)), f())));
  assert(!is_f(call(lambda(var(0)), t())));
  assert(is_f(wrap(call(lambda(var(0)), var(1)), pair(f(), pair(f(), f())))));
  assert(!is_f(wrap(call(lambda(var(0)), var(1)), pair(f(), pair(t(), f())))));
  assert(is_f(wrap(call(lambda(var(1)), f()), pair(f(), f()))));
  assert(!is_f(wrap(call(lambda(var(1)), f()), pair(t(), f()))));
  // evaluation of lists (pairs)
  assert(is_f(first(pair(f(), f()))));
  assert(!is_f(first(pair(t(), f()))));
  assert(is_f(rest(pair(f(), f()))));
  assert(!is_f(rest(pair(f(), t()))));
  assert(!is_f(empty(f())));
  assert(is_f(empty(pair(f(), f()))));
  assert(is_f(at(pair(f(), pair(f(), pair(f(), f()))), 2)));
  assert(!is_f(at(pair(f(), pair(f(), pair(t(), f()))), 2)));
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
  // Y-combinator
  int last = y_comb(lambda(op_if(empty(rest(var(0))), first(var(0)), call(var(1), rest(var(0))))));
  assert(is_f(call(last, pair(f(), f()))));
  assert(!is_f(call(last, pair(t(), f()))));
  assert(is_f(call(last, pair(f(), pair(f(), f())))));
  assert(!is_f(call(last, pair(f(), pair(t(), f())))));
  // strings
  assert(is_f_(str_to_list("")));
  assert(!is_f_(str_to_list("s")));
  assert(num_to_int_(first_(str_to_list("s"))) == 's');
  assert(!strcmp(list_to_str_(str_to_list("str")), "str"));
  assert(!strcmp(list_to_str(call(lambda(pair(var(0), pair(var(0), f()))), int_to_num('x'))), "xx"));
  // number comparison
  assert(is_f(eq_num(int_to_num(5), int_to_num(7))));
  assert(is_f(eq_num(int_to_num(7), int_to_num(5))));
  assert(is_f(eq_num(int_to_num(7), int_to_num(13))));
  assert(is_f(eq_num(int_to_num(13), int_to_num(7))));
  assert(!is_f(eq_num(int_to_num(0), int_to_num(0))));
  assert(!is_f(eq_num(int_to_num(7), int_to_num(7))));
  // identity function
  assert(is_f(call(id(), f())));
  assert(!is_f(call(id(), t())));
  // map
  assert(is_f(map(f(), id())));
  assert(is_f(at(map(pair(f(), f()), id()), 0)));
  assert(!is_f(at(map(pair(t(), f()), id()), 0)));
  int not_fun = lambda(op_not(var(0)));
  assert(!is_f(at(map(pair(f(), f()), not_fun), 0)));
  assert(is_f(at(map(pair(t(), f()), not_fun), 0)));
  assert(!is_f(at(map(pair(f(), pair(f(), f())), not_fun), 1)));
  assert(is_f(at(map(pair(f(), pair(t(), f())), not_fun), 1)));
  // select_if
  assert(!strcmp(list_to_str(select_if(str_to_list("-"), lambda(eq_num(int_to_num('+'), var(0))))), ""));
  assert(!strcmp(list_to_str(select_if(str_to_list("+"), lambda(eq_num(int_to_num('+'), var(0))))), "+"));
  assert(!strcmp(list_to_str(select_if(str_to_list("a+b+"), lambda(eq_num(int_to_num('+'), var(0))))), "++"));
  assert(!strcmp(list_to_str(select_if(str_to_list("a+b+"), lambda(op_not(eq_num(int_to_num('+'), var(0)))))), "ab"));
  // input
  assert(type(input(stdin)) == INPUT);
  assert(is_input(input(stdin)));
  assert(file(input(stdin)) == stdin);
  assert(file(used(input(stdin))) == stdin);
  int in1 = str_to_input("abcdefghijklmnopqrstuvwxyz");
  assert(!is_f(eq_num(first(in1), int_to_num('a'))));
  assert(!is_f(eq_num(first(rest(in1)), int_to_num('b'))));
  assert(!strcmp(list_to_str(in1), "abcdefghijklmnopqrstuvwxyz"));
  // bits to bytes
  assert(!strcmp(list_to_str(bits_to_bytes(pair(f(), pair(t(), pair(f(), f()))))), "010"));
  // bytes to bits
  assert(is_f(at(bytes_to_bits(str_to_list("010")), 0)));
  assert(!is_f(at(bytes_to_bits(str_to_list("010")), 1)));
  assert(is_f(at(bytes_to_bits(str_to_list("010")), 2)));
  assert(is_f(rest(rest(rest(bytes_to_bits(str_to_list("010")))))));
  // select binary
  assert(!strcmp(list_to_str(select_binary(str_to_list("0a1b0"))), "010"));
  fprintf(stderr, "%d\n", cell(VAR));
  destroy();
  return 0;
}
