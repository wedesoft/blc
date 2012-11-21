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

#define BUFSIZE 1024

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
    fprintf(stderr, "Result of parsing is \"%s\" but should be \"%s\"\n", result, spec);
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
  retval = retval | test_io("0010", "0010");
  retval = retval | test_io("00 10", "0010");
  retval = retval | test_io("0100100010", "0100100010");
  retval = retval | test_io(" 01 0010 0010", "0100100010");
#endif
  return retval;
}

