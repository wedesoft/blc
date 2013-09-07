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
#include <string.h>
#include "lambda_bison.h"
extern int yyretval;
%}

%option noyywrap
%option always-interactive

ASCLEAD [a-zA-Z_]
ASCTAIL [a-zA-Z0-9_?!]
U       [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]

UTFLEAD  {ASCLEAD}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}
UTFTAIL  {ASCTAIL}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}

%%

"0"                          { return ZERO; }
"1"                          { return ONE; }
"->"                         { return LAMBDA_; }
"("                          { return LP; }
")"                          { return RP; }
"."                          { return DOT; }
[ \t\n]+                     {}
{UTFLEAD}{UTFTAIL}*[ \t]*"=" { strcpy(yylval.sym, yytext);
                                char *p = yylval.sym;
                                strsep(&p, " \t=");
                                return DEF; }
{UTFLEAD}{UTFTAIL}*          { strcpy(yylval.sym, yytext); return VAR_; }
<<EOF>>                      { return 0; }
.                            {}

%%

