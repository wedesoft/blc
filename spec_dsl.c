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

int first_bit_;
int first_bit(int list)
{
  return call(first_bit_, list);
}

int rest_bits_;
int rest_bits(int list)
{
  return call(rest_bits_, list);
}

int plus(int a, int b)
{
  int fun = y_comb(lambda(lambda(lambda(op_if(op_and(empty(var(0)), empty(var(1))), op_if(var(2), pair(t(), f()), f()),
                              pair(op_xor(op_xor(first_bit(var(0)), first_bit(var(1))), var(2)),
                                   call(call(call(var(3),
                                                  op_if(var(2),
                                                        op_or(first_bit(var(0)), first_bit(var(1))),
                                                        op_and(first_bit(var(0)), first_bit(var(1))))),
                                             rest_bits(var(1))), rest_bits(var(0)))))))));
  return call(call(call(fun, f()), b), a);
}

void init_lib(void)
{
  first_bit_ = lambda(op_if(empty(var(0)), f(), first(var(0))));
  rest_bits_ = lambda(op_if(empty(var(0)), f(), rest(var(0))));
}

int main(void)
{
  init();
  init_lib();
  int n = cell(VAR);
  // Integer addition
  assert(num_to_int(plus(int_to_num(2), int_to_num(1))) == 3);
  assert(num_to_int(plus(int_to_num(2), int_to_num(2))) == 4);
  assert(num_to_int(plus(int_to_num(1), int_to_num(3))) == 4);
  fprintf(stderr, "Test suite requires %d cells.\n", cell(VAR) - n - 1);
  destroy();
  return 0;
}
