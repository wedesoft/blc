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

int test_eval(char *cmd, char *spec)
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
  retval = retval | test_eval("000010", "\n");
  retval = retval | test_eval("00010110 000010 000010", "0\n");
  retval = retval | test_eval("->first.->second.second", "\n");
  retval = retval | test_eval("->first->second.second", "\n");
  retval = retval | test_eval("->first second.second", "\n");
  retval = retval | test_eval("->.->second.second", "\n");
  retval = retval | test_eval("->->second.second", "\n");
  retval = retval | test_eval("(->x.x ->x->y.y)", "\n");
  retval = retval | test_eval("f=->->y.y f", "\n");
  retval = retval | test_eval("->x->y.second=y second", "\n");
  retval = retval | test_eval("f=->->y.y\nt=->x->.x\npair=->h t.->s.(s h t)\n(pair f (pair t f))", "01\n");
  retval = retval | test_eval("->s.(s (input ->x->.x) ->->y.y)\n0", "0\n");
  retval = retval | test_eval("->s.(s (input ->x->.x) ->->y.y)\n1", "1\n");
  retval = retval | test_eval("->s.(s ((input ->->y.y) ->x->.x) ->->y.y)\n00", "0\n");
  retval = retval | test_eval("->s.(s ((input ->->y.y) ->x->.x) ->->y.y)\n01", "1\n");
  return retval;
}

