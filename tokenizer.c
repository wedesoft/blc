/* Bracket - ...
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
#include <stdlib.h>
#include "tokenizer.h"

char *read_token(char *buffer, FILE *stream)
{
  int length = 0;
  char *p = buffer;
  int quit = 0;
  while (!quit) {
    int c = fgetc(stream);
    if (c == EOF) break;
    switch(c) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      if (length > 0) quit = 1;
      break;
    case '(':
    case ')':
      if (length > 0)
        ungetc(c, stream);
      else {
        length = 1;
        *p++ = c;
      };
      quit = 1;
      break;
      break;
    default:
      *p++ = c;
      length += 1;
    };
    if (length > TOKENSIZE) {
      *p = '\0';
      fprintf(stderr,
              "Error: Token %s... longer than %d characters\n",
              buffer,
              TOKENSIZE);
      exit(1);
    };
  }
  *p = '\0';
  return length > 0 ? buffer : NULL;
}

