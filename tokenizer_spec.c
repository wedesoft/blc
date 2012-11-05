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

int test(const char *a, const char *b)
{
  int retval = 0;
  if (!a) a = "NULL";
  if (!b) b = "NULL";
  if (strcmp(a, b)) {
    fprintf(stderr, "Token was \"%s\" but should be \"%s\"\n", a, b);
    retval = 1;
  };
  return retval;
}

int main(void)
{
  int retval = 0;
  char *buf = "(quote (+ a b))";
  FILE *f = fmemopen(buf, strlen(buf), "r");
  char buffer[TOKENSIZE + 2];
  retval = retval | test(read_token(buffer, f), "(");
  retval = retval | test(read_token(buffer, f), "quote");
  retval = retval | test(read_token(buffer, f), "(");
  retval = retval | test(read_token(buffer, f), "+");
  retval = retval | test(read_token(buffer, f), "a");
  retval = retval | test(read_token(buffer, f), "b");
  retval = retval | test(read_token(buffer, f), ")");
  retval = retval | test(read_token(buffer, f), ")");
  retval = retval | test(read_token(buffer, f), NULL);
  fclose(f);
  return retval;
}

