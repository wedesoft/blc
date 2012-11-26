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
#include "blc.h"

#define BUFSIZE 102

int lift_free_vars(int expr, int amount, int depth);
int subst(int lambda, int replacement, int depth);

#ifdef HAVE_FMEMOPEN
int from_string(char *str)
{
  FILE *f = fmemopen(str, strlen(str), "r");
  int retval = read_expr(f);
  fclose(f);
  return retval;
}

char *to_string(char *buffer, int bufsize, int i)
{
  FILE *f = fmemopen(buffer, bufsize, "w");
  print_expr(i, f);
  fclose(f);
  return buffer;
}

int test_io(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, from_string(cmd));
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of parsing \"%s\" is \"%s\" but should be \"%s\"\n", cmd, result, spec);
    retval = 1;
  };
  return retval;
}

int test_lift(char *cmd, int amount, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, lift_free_vars(from_string(cmd), amount, 0));
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result for lifting \"%s\" by %d is \"%s\" but should be \"%s\"\n",
            cmd, amount, result, spec);
    retval = 1;
  };
  return retval;
}

int test_subst(char *lambda, char *arg, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, subst(from_string(lambda), from_string(arg), 0));
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of substituting \"01\" in \"%s\" with \"%s\" is \"%s\" but should be \"%s\"\n",
            lambda, arg, result, spec);
    retval = 1;
  };
  return retval;
}

int test_eval(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, eval_expr(from_string(cmd)));
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of evaluating \"%s\" is \"%s\" but should be \"%s\"\n", cmd, result, spec);
    retval = 1;
  };
  return retval;
}
#endif

int main(void)
{
  int retval = 0;
#ifdef HAVE_FMEMOPEN
  retval = retval | test_io("10", "10");
  retval = retval | test_io("  10 ", "10");
  retval = retval | test_io("110", "110");
  retval = retval | test_io("1110", "1110");
  retval = retval | test_io("  1110 ", "1110");
  retval = retval | test_io("0010", "0010");
  retval = retval | test_io("00 10", "0010");
  retval = retval | test_io("0100100010", "0100100010");
  retval = retval | test_io(" 01 0010 0010", "0100100010");
  retval = retval | test_io("0", "#<err>");
  retval = retval | test_io("00", "#<err>");
  retval = retval | test_io("01", "#<err>");
  retval = retval | test_io("1", "#<err>");
  retval = retval | test_lift("10", 1, "110");
  retval = retval | test_lift("110", -1, "10");
  retval = retval | test_lift("10", -1, "#<err>");
  retval = retval | test_lift("0010", 1, "0010");
  retval = retval | test_lift("0010", -1, "0010");
  retval = retval | test_lift("001110", -1, "00110");
  retval = retval | test_lift("00110", -1, "#<err>");
  retval = retval | test_lift("01001000110", 1, "010010001110");
  retval = retval | test_lift("010010001110", -1, "01001000110");
  retval = retval | test_subst("10", "0010", "0010");
  retval = retval | test_subst("110", "0010", "110");
  retval = retval | test_subst("00110", "0010", "000010");
  retval = retval | test_subst("0010", "0010", "0010");
  retval = retval | test_subst("011010", "0010", "0100100010");
  retval = retval | test_subst("01001010", "0010", "0100100010");
  retval = retval | test_eval("10", "10");
  retval = retval | test_eval("110", "110");
  retval = retval | test_eval("0010", "0010");
  retval = retval | test_eval("00 00 10", "000010");
  retval = retval | test_eval("00 00 110", "0000110");
  retval = retval | test_eval("01 0010 1110", "1110");
  retval = retval | test_eval("01 0000110 0010", "000010");
  retval = retval | test_eval("01 01 000010 0000110 000010", "000010");
  retval = retval | test_eval("01 01 0000110 0000110 000010", "0000110");
  retval = retval | test_eval("01 01 000010 110 10", "10");
  retval = retval | test_eval("01 01 0000110 110 10", "110");
  retval = retval | test_eval("01 01 000010 001110 00110", "00110");
  retval = retval | test_eval("01 01 0000110 001110 00110", "001110");
  retval = retval | test_eval("01 00110 0010", "10");
  retval = retval | test_eval("0100100010", "0010");
#else
  fprintf(stderr, "Cannot run tests without 'fmemopen'!\n");
#endif
  return retval;
}

