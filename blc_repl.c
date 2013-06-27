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
#include "blc.h"

int copy_definitions(int previous_expr, int expression)
{
  int retval;
  gc_push(previous_expr);
  gc_push(expression);
  if (is_definition(previous_expr)) {
    int subexpr = gc_push(copy_definitions(body(previous_expr), expression));
    retval = make_pair(make_definition(term(previous_expr), first(subexpr)), rest(subexpr));
    gc_pop(1);
  } else
    retval = expression;
  gc_pop(2);
  return retval;
}

int main(void)
{
  int previous_expr = NIL;
  while (1) {
    gc_push(previous_expr);
    int input = gc_push(make_input(stdin));
    int output = gc_push(make_output(stdout));
    int expression = gc_push(copy_definitions(previous_expr,
                                              read_expression(input)));
    if (feof(stdin)) break;
    int environment = gc_push(make_pair(rest(expression),
                                        make_pair(output, gc_push(make_false()))));
    print_expression(normalise(eval_expression(first(expression), environment),
                               NIL, 0, 2), stdout);
    fputc('\n', stdout);
    previous_expr = first(expression);
    gc_pop(6);
  };
  return 0;
}
