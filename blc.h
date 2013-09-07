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
#ifndef __BLC_H
#define __BLC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

typedef enum { VAR, LAMBDA, CALL, PROC, WRAP, INPUT } type_t;

int cell(int type);

int type(int cell);

int is_var(int cell);
int is_lambda(int cell);
int is_call(int cell);
int is_proc(int cell);
int is_wrap(int cell);
int is_input(int cell);

int idx(int cell);
int body(int cell);
int fun(int cell);
int arg(int cell);
int term(int cell);
int stack(int cell);
int unwrap(int cell);
int context(int cell);
int cache(int cell);
FILE *file(int cell);
int used(int cell);

int var(int idx);
int lambda(int body);
int call(int fun, int arg);

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

int input(FILE *file);

int int_to_num(int integer);

int eval_env(int cell, int env);

int eval(int cell);

int is_f(int cell);

int first(int list);
int rest(int list);
int empty(int list);
int at(int list, int i);

int op_not(int a);
int op_and(int a, int b);
int op_or(int a, int b);
int eq_bool(int a, int b);

int int_to_num(int integer);

int num_to_int_(int number);

int num_to_int(int number);

int y_comb(int fun);

int str_to_list(const char *str);

char *list_to_str_(int list);

char *list_to_str(int list);

int eq_num(int a, int b);

int id(void);

int map(int list, int fun);

int select_if(int list, int fun);

int bits_to_bytes(int bits);

int bytes_to_bits(int bytes);

int select_binary(int list);

void init(void);

void destroy(void);

int str_to_input(const char *text);

int read_expr(int in);

#endif

