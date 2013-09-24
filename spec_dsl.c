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
#include <string.h>
#include "blc.h"

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
int inject_with(int list, int start, int fun) { return call3(inject_, list, start, fun); }
int inject(int list, int fun)
{
  return op_if(empty(list), f(), inject_with(rest(list), first(list), fun));
}

int add_;
int add(int a, int b) { return call3(add_, a, b, f()); }

int sub_;
int sub(int a, int b) { return call3(sub_, a, b, f()); }

int mul_;
int mul(int a, int b) { return call2(mul_, a, b); }

void init_lib(void)
{
  even_ = proc(op_if(empty(var(0)), t(), op_not(first(var(0)))));
  odd_ = proc(op_if(empty(var(0)), f(), first(var(0))));
  shr_ = proc(op_if(empty(var(0)), f(), rest(var(0))));
  shl_ = proc(op_if(empty(var(0)), f(), pair(f(), var(0))));
  zip_ = y_comb(lambda2(op_if(op_and(empty(var(0)), empty(var(1))),
                              f(),
                              pair(pair(odd(var(0)), odd(var(1))),
                                   call2(var(2), shr(var(0)), shr(var(1)))))));
  inject_ = y_comb(lambda3(op_if(empty(var(0)),
                                 var(1),
                                 call3(var(3), rest(var(0)), call2(var(2), var(1), first(var(0))), var(2)))));
  add_ = y_comb(lambda3(op_if(op_and(empty(var(0)), empty(var(1))),
                              op_if(var(2), pair(t(), f()), f()),
                              call(lambda(pair(op_xor(op_xor(odd(var(1)), odd(var(2))), var(3)),
                                               var(0))),
                                   call3(var(3),
                                         shr(var(1)), shr(var(0)),
                                         op_if(var(2),
                                               op_or(odd(var(0)), odd(var(1))),
                                               op_and(odd(var(0)), odd(var(1)))))))));
  sub_ = y_comb(lambda3(op_if(op_and(empty(var(0)), empty(var(1))),
                              op_if(var(2), pair(t(), call3(var(3), shr(var(0)), shr(var(1)), var(2))), f()),
                              call(lambda(op_if(op_xor(op_xor(odd(var(1)), odd(var(2))), var(3)),
                                                pair(t(), var(0)),
                                                op_if(empty(var(0)), f(), pair(f(), var(0))))),
                                   call3(var(3),
                                         shr(var(0)), shr(var(1)),
                                         op_if(var(2),
                                               op_or(even(var(0)), odd(var(1))),
                                               op_and(even(var(0)), odd(var(1)))))))));
  mul_ = y_comb(lambda2(op_if(empty(var(0)),
                              f(),
                              call(lambda(op_if(first(var(1)), add(var(2), var(0)), var(0))),
                                   shl(call2(var(2), var(1), shr(var(0))))))));
}

int main(void)
{
  init();
  init_lib();
  int n = cell(VAR);
  int i, j;
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
  assert(!is_f(inject_with(pair(t(), pair(t(), pair(t(), f()))), t(), proc(lambda(op_and(var(0), var(1)))))));
  assert(is_f(inject_with(pair(t(), pair(t(), pair(f(), f()))), t(), proc(lambda(op_and(var(0), var(1)))))));
  assert(!is_f(inject_with(pair(f(), pair(f(), pair(t(), f()))), f(), proc(lambda(op_or(var(0), var(1)))))));
  assert(is_f(inject_with(pair(f(), pair(f(), pair(f(), f()))), f(), proc(lambda(op_or(var(0), var(1)))))));
  assert(!is_f(inject(pair(t(), pair(t(), pair(t(), f()))), proc(lambda(op_and(var(0), var(1)))))));
  assert(is_f(inject(pair(t(), pair(t(), pair(f(), f()))), proc(lambda(op_and(var(0), var(1)))))));
  assert(!is_f(inject(pair(f(), pair(f(), pair(t(), f()))), proc(lambda(op_or(var(0), var(1)))))));
  assert(is_f(inject(pair(f(), pair(f(), pair(f(), f()))), proc(lambda(op_or(var(0), var(1)))))));
  // Integer addition
  for (i=0; i<5; i++)
    for (j=0; j<5; j++)
      assert(num_to_int(add(int_to_num(i), int_to_num(j))) == i + j);
  // Integer subtraction
  for (i=0; i<5; i++) {
    assert(is_f(sub(int_to_num(i), int_to_num(i))));
    for (j=0; j<5; j++)
      if (i >= j)
        assert(num_to_int(sub(int_to_num(i), int_to_num(j))) == i - j);
  };
  // Integer multiplication
  for (i=0; i<5; i++)
    for (j=0; j<5; j++)
      assert(num_to_int(mul(int_to_num(i), int_to_num(j))) == i * j);
  fprintf(stderr, "Test suite requires %d cells.\n", cell(VAR) - n - 1);
  destroy();
  return 0;
}
