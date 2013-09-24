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

int odd_;
int odd(int list)
{
  return call(odd_, list);
}

int shr_;
int shr(int list)
{
  return call(shr_, list);
}

int shl_;
int shl(int list)
{
  return call(shl_, list);
}

int add_;
int add(int a, int b)
{
  return call3(add_, f(), b, a);
}

int sub_;
int sub(int a, int b)
{
  return call3(sub_, f(), b, a);
}

int mul_;
int mul(int a, int b)
{
  return call2(mul_, b, a);
}

void init_lib(void)
{
  odd_ = proc(op_if(empty(var(0)), f(), first(var(0))));
  shr_ = proc(op_if(empty(var(0)), f(), rest(var(0))));
  shl_ = proc(op_if(empty(var(0)), f(), pair(f(), var(0))));
  add_ = y_comb(lambda3(op_if(op_and(empty(var(0)), empty(var(1))), op_if(var(2), pair(t(), f()), f()),
                              pair(op_xor(op_xor(odd(var(0)), odd(var(1))), var(2)),
                                   call3(var(3),
                                         op_if(var(2),
                                               op_or(odd(var(0)), odd(var(1))),
                                               op_and(odd(var(0)), odd(var(1)))),
                                         shr(var(1)), shr(var(0)))))));
  sub_ = y_comb(lambda3(op_if(op_and(empty(var(0)), empty(var(1))),
                                           f(),
                                    f())));

  mul_ = y_comb(lambda2(op_if(empty(var(0)),
                              f(),
                              call(lambda(op_if(first(var(1)), add(var(2), var(0)), var(0))),
                                   shl(call2(var(2), shr(var(0)), var(1)))))));
}

int main(void)
{
  init();
  init_lib();
  int n = cell(VAR);
  int i, j;
  // Check for even/odd
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
  // Integer addition
  for (i=0; i<5; i++)
    for (j=0; j<5; j++)
      assert(num_to_int(add(int_to_num(i), int_to_num(j))) == i + j);
  // Integer subtraction
  for (i=0; i<10; i++) {
    for (j=0; j<10; j++)
      if (i >= j)
        printf(" %2d(%2d)", num_to_int(sub(int_to_num(i), int_to_num(j))), i - j);
    printf("\n");
  };
  // Integer multiplication
  for (i=0; i<5; i++)
    for (j=0; j<5; j++)
      assert(num_to_int(mul(int_to_num(i), int_to_num(j))) == i * j);
  fprintf(stderr, "Test suite requires %d cells.\n", cell(VAR) - n - 1);
  destroy();
  return 0;
}
