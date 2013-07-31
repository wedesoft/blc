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
  while (!feof(stdin)) {
    int input = gc_push(make_input(stdin));
    int expression = gc_push(read_expression(input));
    write_expression(first(expression), make_pair(rest(expression), make_false()), stdout);
    fputc('\n', stdout);
    gc_pop(3);
  };
  return 0;
}
