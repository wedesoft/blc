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
#include <string.h>
#include "blc.h"

const char *repl(const char *input)
{
  return list_to_str(bits_to_bytes(first_(read_expr(bytes_to_bits(str_to_list(input))))));
}

int main(void)
{
  init();
  int i, j;
  int n = cell(VAR);
  // variable
  assert(type(var(0)) == VAR);
  assert(is_type(var(0), VAR));
  assert(idx(var(1)) == 1);
  // lambda (function)
  assert(type(lambda(var(0))) == LAMBDA);
  assert(is_type(lambda(var(0)), LAMBDA));
  assert(idx(body(lambda(var(1)))) == 1);
  // call
  assert(type(call(lambda(var(0)), var(0))) == CALL);
  assert(is_type(call(lambda(var(0)), var(0)), CALL));
  assert(idx(body(fun(call(lambda(var(1)), var(2))))) == 1);
  assert(idx(arg(call(lambda(var(1)), var(2)))) == 2);
  // false and true
  assert(idx(body(body(f()))) == 0);
  assert(idx(body(body(t()))) == 1);
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
  assert(is_type(proc(lambda(var(0))), PROC));
  assert(idx(term(proc(var(0)))) == 0);
  assert(is_f_(stack(proc_stack(var(0), f()))));
  // output continuation
  assert(type(output()) == OUTPUT);
  assert(is_type(output(), OUTPUT));
  // memoization
  assert(type(memoize(var(0), wrap(f(), f()))) == MEMOIZE);
  assert(is_type(memoize(var(0), wrap(f(), f())), MEMOIZE));
  assert(is_type(target(memoize(var(0), wrap(f(), f()))), WRAP));
  assert(idx(value(memoize(var(0), wrap(f(), f())))) == 0);
  // booleans
  assert(is_f_(f()));
  assert(!is_f_(t()));
  assert(is_f(f()));
  assert(!is_f(t()));
#if 0
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
  // Test even
  for (i=0; i<10; i+=2) {
    assert(!is_f(even(int_to_num(i))));
    assert( is_f(even(int_to_num(i + 1))));
  };
  // Test odd
  for (i=0; i<10; i+=2) {
    assert( is_f(odd(int_to_num(i))));
    assert(!is_f(odd(int_to_num(i + 1))));
  };
  // Shift right
  for (i=0; i<5; i++)
    assert(num_to_int(shr(int_to_num(i))) == (i >> 1));
  // Shift left
  for (i=0; i<5; i++)
    assert(num_to_int(shl(int_to_num(i))) == (i << 1));
  // Test zip
  for (i=0; i<5; i++)
    for (j=0; j<5; j++) {
      assert(num_to_int(map(zip(int_to_num(i), int_to_num(j)), proc(first(var(0))))) == i);
      assert(num_to_int(map(zip(int_to_num(i), int_to_num(j)), proc(rest(var(0))))) == j);
    };
  // Test injection
  assert(!is_f(inject(pair(t(), pair(t(), pair(t(), f()))), t(), proc(lambda(op_and(var(0), var(1)))))));
  assert(is_f(inject(pair(t(), pair(t(), pair(f(), f()))), t(), proc(lambda(op_and(var(0), var(1)))))));
  assert(!is_f(inject(pair(f(), pair(f(), pair(t(), f()))), f(), proc(lambda(op_or(var(0), var(1)))))));
  assert(is_f(inject(pair(f(), pair(f(), pair(f(), f()))), f(), proc(lambda(op_or(var(0), var(1)))))));
  assert(num_to_int(inject(int_to_num(11), f(), proc(lambda(pair(var(1), var(0)))))) == 13);
  // Test left fold
  assert(!is_f(foldleft(pair(t(), pair(t(), pair(t(), f()))), t(), proc(lambda(op_and(var(0), var(1)))))));
  assert(is_f(foldleft(pair(t(), pair(t(), pair(f(), f()))), t(), proc(lambda(op_and(var(0), var(1)))))));
  assert(!is_f(foldleft(pair(f(), pair(f(), pair(t(), f()))), f(), proc(lambda(op_or(var(0), var(1)))))));
  assert(is_f(foldleft(pair(f(), pair(f(), pair(f(), f()))), f(), proc(lambda(op_or(var(0), var(1)))))));
  assert(num_to_int(foldleft(int_to_num(11), f(), proc(lambda(pair(var(1), var(0)))))) == 11);
  // number comparison
  assert(is_f(eq_num(int_to_num(5), int_to_num(7))));
  assert(is_f(eq_num(int_to_num(7), int_to_num(5))));
  assert(is_f(eq_num(int_to_num(7), int_to_num(13))));
  assert(is_f(eq_num(int_to_num(13), int_to_num(7))));
  assert(!is_f(eq_num(int_to_num(0), int_to_num(0))));
  assert(!is_f(eq_num(int_to_num(7), int_to_num(7))));
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
  int alpha = str_to_input("abcdefg");
  assert(!is_f(eq_num(first(alpha), int_to_num('a'))));
  assert(!is_f(eq_num(first(rest(alpha)), int_to_num('b'))));
  assert(!strcmp(list_to_str(alpha), "abcdefg"));
  // bits to bytes
  assert(!strcmp(list_to_str(bits_to_bytes(pair(f(), pair(t(), pair(f(), f()))))), "010"));
  // bytes to bits
  assert(is_f(at(bytes_to_bits(str_to_list("010")), 0)));
  assert(!is_f(at(bytes_to_bits(str_to_list("010")), 1)));
  assert(is_f(at(bytes_to_bits(str_to_list("010")), 2)));
  assert(is_f(rest(rest(rest(bytes_to_bits(str_to_list("010")))))));
  // select binary
  assert(!strcmp(list_to_str(select_binary(str_to_list("0a1b0"))), "010"));
  // bit input
  int bit_input = str_to_input("01 000010 10 110");
  assert(!strcmp(list_to_str(bits_to_bytes(bytes_to_bits(select_binary(bit_input)))),
                 "0100001010110"));
  // read variable
  assert(is_var(first_(read_expr(bytes_to_bits(str_to_list("10"))))));
  assert(idx(first_(read_expr(bytes_to_bits(str_to_list("1110"))))) == 2);
  assert(!is_f(empty(rest_(read_expr(bytes_to_bits(str_to_list("1110")))))));
  // read lambda
  assert(is_lambda(first_(read_expr(bytes_to_bits(str_to_list("0010"))))));
  assert(idx(body(first_(read_expr(bytes_to_bits(str_to_list("00110")))))) == 1);
  assert(!is_f(empty(rest_(read_expr(bytes_to_bits(str_to_list("00110")))))));
  // read call
  assert(is_call(first_(read_expr(bytes_to_bits(str_to_list("0100100010"))))));
  assert(idx(body(fun(first_(read_expr(bytes_to_bits(str_to_list("01001101110"))))))) == 1);
  assert(idx(arg(first_(read_expr(bytes_to_bits(str_to_list("01001101110")))))) == 2);
  assert(!is_f(empty(rest_(read_expr(bytes_to_bits(str_to_list("0100100010")))))));
  // evaluation of input
  assert(!strcmp(repl("000010"), ""));
  assert(!strcmp(repl("00010110000010000010"), "0"));
  assert(!strcmp(repl("000101100000110000010"), "1"));
  assert(!strcmp(repl("00010110000011000010110000010000010"), "10"));
#endif
  fprintf(stderr, "Test suite requires %d cells.\n", cell(VAR) - n - 1);
  destroy();
  return 0;
}
