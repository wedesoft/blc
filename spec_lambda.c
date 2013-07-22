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
#include <string.h>
#include "blc.h"
#include "lambda.h"

#define BUFSIZE 256

int test_compile(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = buffer;
  FILE *f_in = fmemopen(cmd, strlen(cmd), "r");
  FILE *f_out = fmemopen(result, BUFSIZE, "w");
  compile_lambda(f_in, f_out);
  fclose(f_out);
  fclose(f_in);
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of compiling \"%s\" is \"%s\" but should be \"%s\"\n", cmd, result, spec);
    retval = 1;
  };
  return retval;
}

int main(void)
{
  int retval = 0;
  retval = retval | test_compile("0010", "0010\n");
  retval = retval | test_compile("->x.x", "0010\n");
  retval = retval | test_compile("->xy.xy", "0010\n");
  retval = retval | test_compile("-> x.x", "0010\n");
  retval = retval | test_compile("->x .x", "0010\n");
  retval = retval | test_compile("->x. x", "0010\n");
  retval = retval | test_compile("->x.->y.x", "0000110\n");
  retval = retval | test_compile("->x.->y.y", "000010\n");
  retval = retval | test_compile("->x.->x.x", "000010\n");
  retval = retval | test_compile("->.->y.y", "000010\n");
  retval = retval | test_compile("->->y.y", "000010\n");
  retval = retval | test_compile("->x->y.x", "0000110\n");
  retval = retval | test_compile("->x y.x", "0000110\n");
  retval = retval | test_compile("->x.->y.x", "0000110\n");
  retval = retval | test_compile("->x->y.x", "0000110\n");
  retval = retval | test_compile("->x->.x", "0000110\n");
  retval = retval | test_compile("-> x -> y . x", "0000110\n");
  retval = retval | test_compile("f=->->x.x f", "000010\n");
  retval = retval | test_compile("f= ->->x.x f", "000010\n");
  retval = retval | test_compile("f =->->x.x f", "000010\n");
  retval = retval | test_compile("->x.y=x y", "0001001010\n");
  retval = retval | test_compile("(->->x.x ->y.y)", "0010\n");
  retval = retval | test_compile("(->->x.x ->x.x)", "0010\n");
  retval = retval | test_compile("((->x->y.x input) output)", "10\n");
  retval = retval | test_compile("((->x->y.y input) output)", "110\n");
  retval = retval | test_compile("(->x->y.x input output)", "10\n");
  retval = retval | test_compile("(->x->y.y input output)", "110\n");
  retval = retval | test_compile("(input ->x->y.x)\n0", "000010\n");
  retval = retval | test_compile("(input ->x->y.x)\n1", "0000110\n");
  retval = retval | test_compile("(input)", "10\n");
  return retval;
}

