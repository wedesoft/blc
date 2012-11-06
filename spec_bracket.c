/* Bracket - Attempt at writing a small Racket (Scheme) interpreter
 * Copyright (C) 2012  Jan Wedekind
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
#include "tokenizer.h"
#include "boot-eval.h"

#define BUFSIZE 1024

int from_string(char *str)
{
  FILE *f = fmemopen(str, strlen(str), "r");
  int retval = read_expression(f);
  fclose(f);
  return retval;
}

char *to_string(char *buffer, int bufsize, int i)
{
  FILE *f = fmemopen(buffer, bufsize, "w");
  print_expression(i, f);
  fclose(f);
  return buffer;
}

int test(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, from_string(cmd));
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result was \"%s\" but should be \"%s\"\n", result, spec);
    retval = 1;
  };
  return retval;
}

int main(void)
{
  int retval = 0;
  test("null", "null");
  test(" (  cons \n x  y  )\n", "(cons x y)");
  test("((lambda x y)7 )", "((lambda x y) 7)");
  test("(quote ())", "(quote ())");
  return retval;
}
