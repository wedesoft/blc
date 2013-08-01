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

#define BUFSIZE 102

int str_to_list(char *str)
{
  int retval;
  if (*str == '\0')
    retval = make_false();
  else if (*str == '0') {
    retval = make_pair(gc_push(make_false()), gc_push(str_to_list(str + 1)));
    gc_pop(2);
  } else if (*str == '1') {
    retval = make_pair(gc_push(make_true()), gc_push(str_to_list(str + 1)));
    gc_pop(2);
  } else
    retval = str_to_list(str + 1);
  return retval;
}

int from_string(char *str)
{
  return read_expression(str_to_list(str));
}

char *to_string(char *buffer, int bufsize, int expression)
{
  gc_push(expression);
  FILE *f = fmemopen(buffer, bufsize, "w");
  write_expression(first(expression), make_pair(rest(expression), make_false()), f);
  fclose(f);
  gc_pop(1);
  return buffer;
}

int test_eval(char *command, char *specification)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, from_string(command));
  if (strcmp(specification, result)) {
    fprintf(stderr, "Result of evaluating \"%s\" is \"%s\" but should be \"%s\"\n", command, result, specification);
    retval = 1;
  };
  assert(n_registers == 0);
  return retval;
}

int main(void)
{
  int retval = 0;
  retval = retval | test_eval("00 00 10", ""); // empty list
  retval = retval | test_eval("00010110 000010 000010", "0"); // list with false
  retval = retval | test_eval("00010110 0000110 000010", "1"); // list with true
  retval = retval | test_eval("00010110 000010 00010110 000010 000010", "00"); // two element list
  retval = retval | test_eval("00010110 000010 00010110 0000110 000010", "01"); // two element list
  retval = retval | test_eval("00010110 01 0010 000010 000010", "0"); // identity function
  retval = retval | test_eval("00010110 01 0010 0000110 000010", "1"); // identity function
  retval = retval | test_eval("00010110 01 01 000010 000010 0000110 000010", "1"); // not false
  retval = retval | test_eval("00010110 01 01 0000110 000010 0000110 000010", "0"); // not true
  retval = retval | test_eval("10 100001", "100001"); // mirror input
  retval = retval | test_eval("00010110 011100000110 000010 0", "0"); // read false
  retval = retval | test_eval("00010110 011100000110 000010 1", "1"); // read true
  retval = retval | test_eval("00010110 01011100000100000110 000010 00", "0"); // read second bit
  retval = retval | test_eval("00010110 01011100000100000110 000010 01", "1"); // read second bit
  return retval;
}

