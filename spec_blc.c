/* Bracket - Binary Lambda Calculus VM and DSL on top of it
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

int from_string(char *str)
{
  FILE *f = fmemopen(str, strlen(str), "r");
  int retval = read_expr(f);
  fclose(f);
  return retval;
}

char *to_string(char *buffer, int bufsize, int expr)
{
  FILE *f = fmemopen(buffer, bufsize, "w");
  print_expr(expr, f);
  fclose(f);
  return buffer;
}

int test_false(char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, make_false());
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of creating \"false\" is \"%s\" but should be \"%s\"\n", result, spec);
    retval = 1;
  };
  return retval;
}

int test_true(char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, make_true());
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of creating \"true\" is \"%s\" but should be \"%s\"\n", result, spec);
    retval = 1;
  };
  return retval;
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

int test_eval(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  int env = make_false();
  char *result = to_string(buffer, BUFSIZE, eval_expr(from_string(cmd), env));
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of evaluating \"%s\" is \"%s\" but should be \"%s\"\n", cmd, result, spec);
    retval = 1;
  };
  return retval;
}

int test_input(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  FILE *f = fmemopen(cmd, strlen(cmd), "r");
  int env = gc_push(cons(gc_push(make_input(f)), gc_push(make_false())));
  char *result = to_string(buffer, BUFSIZE, eval_expr(read_expr(f), env));
  gc_pop(3);
  fclose(f);
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of evaluating \"%s\" is \"%s\" but should be \"%s\"\n", cmd, result, spec);
    retval = 1;
  };
  return retval;
}

int main(void)
{
  int retval = 0;
  retval = retval | test_false("000010");
  retval = retval | test_true("0000110");
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
  retval = retval | test_eval("10", "10");
  retval = retval | test_eval("110", "110");
  retval = retval | test_eval("0010", "#<proc:10;#env=0>");
  retval = retval | test_eval("00 00 10", "#<proc:0010;#env=0>");
  retval = retval | test_eval("00 00 110", "#<proc:00110;#env=0>");
  retval = retval | test_eval("01 10 110", "10");
  retval = retval | test_eval("01 110 10", "110");
  retval = retval | test_eval("01 0010 1110", "1110");
  retval = retval | test_eval("01 000010 0010", "#<proc:10;#env=1>");
  retval = retval | test_eval("01 0000110 0010", "#<proc:110;#env=1>");
  retval = retval | test_eval("01 01 000010 0000110 000010", "#<proc:0010;#env=0>");
  retval = retval | test_eval("01 01 0000110 0000110 000010", "#<proc:00110;#env=0>");
  retval = retval | test_eval("01 01 000010 110 10", "10");
  retval = retval | test_eval("01 00 01 01 10 1110 110 000010", "10");
  retval = retval | test_eval("01 01 0000110 110 10", "110");
  retval = retval | test_eval("01 00 01 01 10 1110 110 0000110", "110");
  retval = retval | test_eval("01 01 000010 001110 00110", "#<proc:110;#env=0>");
  retval = retval | test_eval("01 01 0000110 001110 00110", "#<proc:1110;#env=0>");
  retval = retval | test_eval("01 00110 0010", "10");
  retval = retval | test_eval("0100100010", "#<proc:10;#env=0>");
  retval = retval | test_input("01 10 0000110 0", "#<proc:0010;#env=2>");
  retval = retval | test_input("01 10 0000110 1", "#<proc:00110;#env=2>");
  retval = retval | test_input("01 01 10 000010 0000110 00", "#<proc:0010;#env=3>");
  retval = retval | test_input("01 01 10 000010 0000110 01", "#<proc:00110;#env=3>");
  retval = retval | test_input("01 01 01 10 0000110 1110 110 0", "10");
  retval = retval | test_input("01 01 01 10 0000110 1110 110 1", "110");
  retval = retval | test_input("01 01 01 01 10 000010 0000110 1110 110 00", "10");
  retval = retval | test_input("01 01 01 01 10 000010 0000110 1110 110 01", "110");
  return retval;
}

