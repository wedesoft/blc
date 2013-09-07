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
#include <stdlib.h>
#include "blc.h"

int main(void)
{
  init();
  int expr = read_expr(bytes_to_bits(select_binary(input(stdin))));
  int list = eval_env(bits_to_bytes(first_(expr)), pair(rest_(expr), f()));
  while (is_f(empty(list))) {
    fputc(num_to_int(first(list)), stdout);
    list = eval(rest(list));
  };
  fputc('\n', stdout);
  destroy();
  return 0;
}
