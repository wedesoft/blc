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
#include <string.h>

#define TOKENSIZE 8

char *read_token(char *str)
{
  char *retval = fgets(str, TOKENSIZE + 3, stdin);
  if (retval) {
    int len = strlen(retval) - 1;
    retval[len] = '\0';
    if (len <= 0) {
      fprintf(stderr, "Error: Encountered empty token.\n");
      exit(1);
    };
    if (len > TOKENSIZE) {
      fprintf(stderr, "Error: Token %s... longer than %d characters.\n",
              retval, TOKENSIZE);
      exit(1);
    };
  };
  return retval;
}

int main(void)
{
  while (1) {
    char line[TOKENSIZE + 3];
    char *str = read_token(line);
    if (!str) break;
    printf("%s\n", str);
  };
  return 0;
}
