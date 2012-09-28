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
#include <stdlib.h>
#include <string.h>

#define PAIRS 128
#define TOKENSIZE 8
#define NIL -1

typedef enum {
  PAIR = 0,
  SYMBOL
} type_t;

typedef struct {
  int car;
  int cdr;
} pair_t;

typedef char symbol_t[TOKENSIZE + 3];

typedef struct {
  type_t type;
  union {
    pair_t pair;
    symbol_t symbol;
  };
} cell_t;

char *read_token(char *str)
{
  char *retval = fgets(str, TOKENSIZE + 3, stdin);
  if (retval) {
    int len = strlen(retval) - 1;
    retval[len] = '\0';
    if (len <= 0) {
      fprintf(stderr, "Error: Encountered empty token\n");
      exit(1);
    };
    if (len > TOKENSIZE) {
      fprintf(stderr, "Error: Token %s... longer than %d characters\n",
              retval, TOKENSIZE);
      exit(1);
    };
  };
  return retval;
}

int n = 0;
cell_t cells[PAIRS];

int parse_list(void)
{
  int retval;
  int cell = parse();
  if (cell != NIL) {
    retval = n++;
    cells[retval].type = PAIR;
    cells[retval].pair.car = cell;
    cells[retval].pair.cdr = parse_list();
  } else {
    retval = -1;
  };
  return retval;
}

int parse(void)
{
  int retval = n;
  char *str = read_token(cells[retval].symbol);
  if (!str) {
    fprintf(stderr, "Error: Unexpected end of input\n");
    exit(1);
  };
  switch (str[0]) {
  case '(':
    n++;
    cells[retval].type = PAIR;
    cells[retval].pair.car = parse_list();
    cells[retval].pair.cdr = NIL;
    break;
  case ')':
    retval = -1;
    break;
  default:
    n++;
    cells[retval].type = SYMBOL;
  };
  return retval;
}

void print(int i);

void print_list(int i)
{
  if (cells[i].type == PAIR) {
    print(cells[i].pair.car);
    if (cells[i].pair.cdr != NIL) {
      fputc(' ', stdout);
      print_list(cells[i].pair.cdr);
    }
  };
}

void print(int i)
{
  if (cells[i].type == PAIR) {
    fputc('(', stdout);
    print_list(cells[i].pair.car);
    fputc(')', stdout);
  } else {
    char *symbol = cells[i].symbol;
    while (*symbol) fputc(*symbol++, stdout);
  }
}

int main(void)
{
  print(parse());
  fputc('\n', stdout);
  return 0;
}

