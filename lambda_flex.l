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
%{
#include <stdio.h>
#include "lambda_bison.h"
extern int yyretval;
%}

%option noyywrap

%%

"0"                  { return ZERO; }
"1"                  { return ONE; }
"->"|"λ"             { return LAMBDA; }
"("                  { return LP; }
")"                  { return RP; }
"."                  { return DOT; }
[a-zA-Z][a-zA-Z0-9]* { strcpy(yylval.sym, yytext); return VAR; }
<<EOF>>              { return 0; }
[ \t\n]+             {}
.                    {}

%%

