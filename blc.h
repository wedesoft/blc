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
#ifndef __BLC_H
#define __BLC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, MEMOIZE, INPUT, OUTPUT } type_t;

int cell(int type);

#ifndef NDEBUG
int tag(int cell, const char *value);
#endif

int type(int cell);

int is_type(int cell, int t);

int idx(int cell);
int body(int cell);
int fun(int cell);
int arg(int cell);
int term(int cell);
int stack(int cell);
int unwrap(int cell);
int context(int cell);
int cache(int cell);
int value(int cell);
int target(int cell);
FILE *file(int cell);
int used(int cell);

int var(int idx);
int lambda(int body);
int lambda2(int body);
int lambda3(int body);
int call(int fun, int arg);
int call2(int fun, int arg1, int arg2);
int call3(int fun, int arg1, int arg2, int arg3);

int f(void);
int t(void);

int is_f_(int cell);

int op_if(int condition, int consequent, int alternative);

int pair(int first, int rest);

int first_(int list);
int rest_(int list);

int at_(int list, int i);

int proc_stack(int term, int stack);

int proc(int term);

int wrap(int unwrap, int context);

int memoize(int value, int target);

int input(FILE *file);

int output(void);

int reindex(int cell, int index);

int cps_atom(int cell);

int cps_expr(int cell, int cont);

int int_to_num(int integer);

int eval(int cell);

int is_f(int cell);

int first(int list);
int rest(int list);
int empty(int list);
int at(int list, int i);

int op_not(int a);
int op_and(int a, int b);
int op_or(int a, int b);
int op_xor(int a, int b);
int eq_bool(int a, int b);

int int_to_num(int integer);

int num_to_int_(int number);

int num_to_int(int number);

int y_comb(int fun);

int str_to_list(const char *str);

const char *list_to_str_(int list);

const char *list_to_str(int list);

int id(void);

int map(int list, int fun);

int even(int list);

int odd(int list);

int shr(int list);

int shl(int list);

int zip(int a, int b);

int inject(int list, int start, int fun);

int foldleft(int list, int start, int fun);

int eq_num(int a, int b);

int select_if(int list, int fun);

int bits_to_bytes(int bits);

int bytes_to_bits(int bytes);

int select_binary(int list);

void init(void);

void destroy(void);

int str_to_input(const char *text);

int read_expr(int in);

void write_expression(int expr, int env, FILE *stream);

#endif

