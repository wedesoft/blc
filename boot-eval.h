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
#ifndef __BOOT_EVAL_H
#define __BOOT_EVAL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>

extern int environment;

int read_expression(FILE *stream);
int eval_expression(int i, int env);
void print_expression(int i, FILE *stream);
void print_quoted(int i, FILE *stream);
void initialize(void);

#endif

