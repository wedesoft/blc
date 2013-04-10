/* BLC - Binary Lambda Calculus VM and DSL on top of it
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
#include "blc.h"

int main(void)
{
  while (1) {
    int input = gc_push(make_input(stdin));
    int output = gc_push(make_output(stdout));
    int expression = gc_push(read_expression(input));
    if (feof(stdin)) break;
    int environment = gc_push(make_pair(second(expression), make_pair(output, gc_push(make_false()))));
    print_expression(eval_expression(first(expression), environment), stdout);
    fputc('\n', stdout);
    gc_pop(5);
  };
  return 0;
}
