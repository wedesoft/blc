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

int plus_;
int plus(int a, int b)
{
  return call(call(call(plus_, f()), b), a);
}

int mult_;
int mult(int a, int b)
{
  return call(call(mult_, b), a);
}

void init_lib(void)
{
  odd_ = lambda(op_if(empty(var(0)), f(), first(var(0))));
  shr_ = lambda(op_if(empty(var(0)), f(), rest(var(0))));
  shl_ = lambda(op_if(empty(var(0)), f(), pair(f(), var(0))));
  plus_ = y_comb(lambda(lambda(lambda(op_if(op_and(empty(var(0)), empty(var(1))), op_if(var(2), pair(t(), f()), f()),
                    pair(op_xor(op_xor(odd(var(0)), odd(var(1))), var(2)),
                         call(call(call(var(3),
                                        op_if(var(2),
                                              op_or(odd(var(0)), odd(var(1))),
                                              op_and(odd(var(0)), odd(var(1))))),
                                   shr(var(1))), shr(var(0)))))))));
  mult_ = y_comb(lambda(lambda(op_if(empty(var(0)),
                                     f(),
                                     call(lambda(op_if(first(var(1)), plus(var(2), var(0)), var(0))),
                                          shl(call(call(var(2), shr(var(0))), var(1))))))));
}

int main(void)
{
  init();
  init_lib();
  int n = cell(VAR);
  // Check for even/odd
  assert(is_f(odd(int_to_num(4))));
  assert(!is_f(odd(int_to_num(5))));
  // Shift right
  assert(num_to_int(shr(int_to_num(6))) == 3);
  assert(num_to_int(shr(int_to_num(5))) == 2);
  // Shift left
  assert(num_to_int(shl(int_to_num(6))) == 12);
  assert(num_to_int(shl(int_to_num(5))) == 10);
  // Integer addition
  assert(num_to_int(plus(int_to_num(0), int_to_num(1))) == 1);
  assert(num_to_int(plus(int_to_num(2), int_to_num(1))) == 3);
  assert(num_to_int(plus(int_to_num(2), int_to_num(2))) == 4);
  assert(num_to_int(plus(int_to_num(1), int_to_num(3))) == 4);
  // Integer multiplication
  assert(num_to_int(mult(int_to_num(0), int_to_num(2))) == 0);
  assert(num_to_int(mult(int_to_num(4), int_to_num(1))) == 4);
  assert(num_to_int(mult(int_to_num(4), int_to_num(2))) == 8);
  assert(num_to_int(mult(int_to_num(5), int_to_num(3))) == 15);
  fprintf(stderr, "Test suite requires %d cells.\n", cell(VAR) - n - 1);
  destroy();
  return 0;
}
