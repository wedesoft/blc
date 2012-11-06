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

int test_token(FILE *f, const char *spec)
{
  int retval = 0;
  char buffer[TOKENSIZE + 3];
  char *result = read_token(buffer, f);
  if (!result) result = "NULL";
  if (!spec) spec = "NULL";
  if (strcmp(result, spec)) {
    fprintf(stderr, "Token was \"%s\" but should be \"%s\"\n", result, spec);
    retval = 1;
  };
  return retval;
}

int test_read_print(char *cmd, char *spec)
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
  {
    char *buf = "(quote \n  (+ a b))";
    FILE *f = fmemopen(buf, strlen(buf), "r");
    retval = retval | test_token(f, "(");
    retval = retval | test_token(f, "quote");
    retval = retval | test_token(f, "(");
    retval = retval | test_token(f, "+");
    retval = retval | test_token(f, "a");
    retval = retval | test_token(f, "b");
    retval = retval | test_token(f, ")");
    retval = retval | test_token(f, ")");
    retval = retval | test_token(f, NULL);
    fclose(f);
  };
  {
    retval = retval | test_read_print("null", "null");
    retval = retval | test_read_print(" (  cons \n x  y  )\n", "(cons x y)");
    retval = retval | test_read_print("((lambda\tx\ty)7 )", "((lambda x y) 7)");
    retval = retval | test_read_print("(quote ())", "(quote ())");
  };
  return retval;
}
