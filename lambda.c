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
#include <ctype.h>
#include <string.h>
#include "blc.h"
#include "lambda.h"

#define STACKSIZE  16384
#define NAMEBUFSIZE 65536
#define TOKENSIZE 16

typedef enum {QUIT = 0, INIT, ZERO, ONE, MINUS, LAMBDA, DOT, SETVAR, GETVAR} state_t;

int stack[STACKSIZE];
int stack_n;
char names[NAMEBUFSIZE];
char *name_p;
char token[TOKENSIZE];
char *token_p;

int find_var(const char *token)
{
  int retval = -1;
  int n = 0;
  char *p = names;
  while (p < name_p) {
    if (strcmp(p, token)) {
      p += strlen(p) + 1;
      n++;
    } else {
      retval = n;
      break;
    }
  }
  return retval;
}

int depth(void)
{
  int retval = 0;
  char *p = names;
  while (p < name_p) {
    retval++;
    p += strlen(p) + 1;
  };
  return retval;
}

int compile_lambda(FILE *f_in, FILE *f_out)
{
  int retval = 0;
  stack_n = 0;
  state_t state = INIT;
  name_p = names;
  while (state) {
    int c = fgetc(f_in);
    switch (state) {
    case INIT:
      if (isalpha(c)) {
        token_p = token;
        *token_p++ = c;
        state = GETVAR;
      } else
        switch (c) {
        case '-':
          state = MINUS;
          break;
        case '0':
          fputc('0', f_out);
          state = ZERO;
          break;
        case '1':
          fputc('1', f_out);
          state = ONE;
          break;
        case EOF:
          state = QUIT;
          break;
        default:
          fputc(c, f_out);
        };
      break;
    case ZERO:
      switch (c) {
      case '0':
        fputc('0', f_out);
        *name_p++ = '\0';
        state = INIT;
        break;
      case '1':
        fputc('1', f_out);
        stack[stack_n++] = depth();
        state = INIT;
        break;
      default:
        fputc(c, f_out);
      };
      break;
    case ONE:
      switch (c) {
      case '0':
        fputc('0', f_out);
        state = INIT;
        break;
      default:
        fputc(c, f_out);
      };
      break;
    case MINUS:
      switch (c) {
      case '-':
        fputc('-', f_out);
        break;
      case '>':
        fputs("00", f_out);
        state = LAMBDA;
        break;
      case '0':
        fputc('-', f_out);
        fputc('0', f_out);
        state = ZERO;
        break;
      case '1':
        fputc('-', f_out);
        fputc('1', f_out);
        state = ONE;
        break;
      case EOF:
        fputc('-', f_out);
        state = QUIT;
        break;
      default:
        fputc('-', f_out);
        fputc(c, f_out);
        state = INIT;
      };
      break;
    case LAMBDA:
      if (isalpha(c)) {
        *name_p++ = c;
        state = SETVAR;
      } else if (isblank(c))
        fputc(c, f_out);
      else {
        *name_p++ = '\0';
        switch (c) {
        case EOF:
          state = QUIT;
          break;
        case '.':
          state = INIT;
          break;
        case '-':
          state = MINUS;
          break;
        case '0':
          fputc('0', f_out);
          state = ZERO;
          break;
        case '1':
          fputc('1', f_out);
          state = ONE;
          break;
        default:
          fputc(c, f_out);
          state = INIT;
        };
      };
      break;
    case DOT:
      if (isalpha(c)) {
        *token_p++ = c;
        state = GETVAR;
      } else if (isblank(c))
        fputc(c, f_out);
      else
        switch (c) {
        case EOF:
          state = QUIT;
          break;
        case '.':
          state = INIT;
          break;
        case '-':
          state = MINUS;
          break;
        case '0':
          fputc('0', f_out);
          state = ZERO;
          break;
        case '1':
          fputc('1', f_out);
          state = ONE;
          break;
        default:
          fputc(c, f_out);
          state = INIT;
        };
      break;
    case SETVAR:
      if (isalpha(c))
        *name_p++ = c;
      else {
        *name_p++ = '\0';
        switch (c) {
        case EOF:
          state = QUIT;
          break;
        case '-':
          state = MINUS;
          break;
        case '.':
          state = INIT;
          break;
        case '0':
          fputc('0', f_out);
          state = ZERO;
          break;
        case '1':
          fputc('1', f_out);
          state = ONE;
          break;
        default:
          fputc(c, f_out);
          state = DOT;
        };
      };
      break;
    case GETVAR:
      if (isalpha(c))
        *token_p++ = c;
      else {
        int var;
        *token_p = '\0';
        var = find_var(token);
        if (var == -1)
          fputs(token, f_out);
        else
          print_var(var, f_out);
        switch (c) {
        case EOF:
          state = QUIT;
          break;
        case '0':
          fputc('0', f_out);
          state = ZERO;
          break;
        case '1':
          fputc('1', f_out);
          state = ONE;
          break;
        default:
          fputc(c, f_out);
          state = INIT;
        };
      };
    default:
      break;
    };
    if (c == '\n') fflush(f_out);
  };
  return retval;
}

