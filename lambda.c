/* BLC - Binary Lambda Calculus interpreter
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
#include "lambda.h"
#include "lambda_bison.h"

#define NAMEBUFSIZE 65536

extern int names;
extern char buffer[NAMEBUFSIZE];
extern char *buffer_p;

extern void yyrestart(FILE *input_file);
extern void yyset_out(FILE *out_str);
extern int yyparse(void);

int yyretval;

int compile_lambda(FILE *f_in, FILE *f_out)
{
  names = 0;
  buffer_p = buffer;
  yyretval = 0;
  yyrestart(f_in);
  yyset_out(f_out);
  yyparse();
  return yyretval;
}

