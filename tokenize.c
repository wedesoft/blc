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
#include <stdio.h>

int main(void)
{
  int length = 0;
  while (1) {
    int c = fgetc(stdin);
    if (c == EOF) break;
    switch(c) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      if (length > 0) fputc('\n', stdout);
      length = 0;
      break;
    case '(':
    case ')':
      if (length > 0) fputc('\n', stdout);
      length = 0;
      fputc(c, stdout);
      fputc('\n', stdout);
      break;
    default:
      length += 1;
      fputc(c, stdout);
    };
    fflush(stdout);
  };
  if (length > 0) fputc('\n', stdout);
  return 0;
}

