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
#include <assert.h>
#include <string.h>
#include "boot-eval.h"

#define BUFSIZE 1024

#ifdef HAVE_FMEMOPEN
int from_string(char *str)
{
  FILE *f = fmemopen(str, strlen(str), "r");
  int retval = read_expression(f);
  fclose(f);
  return retval;
}

char *to_string(char *buffer, int bufsize, int i)
{
  FILE *f = fmemopen(buffer, bufsize, "w");
  print_quoted(i, f);
  fclose(f);
  return buffer;
}

int test_token(FILE *f, const char *spec)
{
  int retval = 0;
  char buffer[TOKENSIZE + 3];
  char *result = read_token(buffer, f);
  if (!result) result = "NULL";
  if (!spec) spec = "NULL";
  if (strcmp(result, spec)) {
    fprintf(stderr, "Result of reading token is \"%s\" but should be \"%s\"\n", result, spec);
    retval = 1;
  };
  return retval;
}

int test_io(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, from_string(cmd));
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of parsing is \"%s\" but should be \"%s\"\n", result, spec);
    retval = 1;
  };
  return retval;
}

int test_eval(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = to_string(buffer, BUFSIZE, eval_expression(from_string(cmd), environment));
  if (strcmp(spec, result)) {
    fprintf(stderr, "Evaluating \"%s\" resulted in \"%s\" but should be \"%s\"\n", cmd, result, spec);
    retval = 1;
  };
  return retval;
}
#endif

int main(void)
{
  int retval = 0;
#ifdef HAVE_FMEMOPEN
  {
    char *buf = "(quote \n  (+ a b))";
    FILE *f = fmemopen(buf, strlen(buf), "r");
    retval = retval | test_token(f, "(");
    retval = retval | test_token(f, "quote");
    retval = retval | test_token(f, "(");
    retval = retval | test_token(f, "+");
    retval = retval | test_token(f, "a");
    retval = retval | test_token(f, "b");
    retval = retval | test_token(f, ")");
    retval = retval | test_token(f, ")");
    retval = retval | test_token(f, NULL);
    fclose(f);
  };
  {
    retval = retval | test_io("null", "(quote null)");
    retval = retval | test_io(" (  cons \n x  y  )\n", "(quote (cons x y))");
    retval = retval | test_io("((lambda\tx\ty)7 )", "(quote ((lambda x y) 7))");
    retval = retval | test_io("(quote ())", "(quote (quote ()))");
    retval = retval | test_io("(1 . ())", "(quote (1))");
    retval = retval | test_io("(1 . 2)", "(quote (1 . 2))");
  };
  {
    initialize();
    retval = retval | test_eval("(quote ())", "(quote ())");
    retval = retval | test_eval("null", "(quote ())");
    retval = retval | test_eval("(quote (+ (* 2 x) 3))", "(quote (+ (* 2 x) 3))");
    retval = retval | test_eval("(first (quote (1 2 3)))", "(quote 1)");
    retval = retval | test_eval("(rest (quote (1 2 3)))", "(quote (2 3))");
    retval = retval | test_eval("(first (rest (quote (1 2 3))))", "(quote 2)");
    retval = retval | test_eval("(cons 1 (quote (2 3)))", "(quote (1 2 3))");
    retval = retval | test_eval("(cons 1 (quote ()))", "(quote (1))");
    retval = retval | test_eval("(cons 1 (cons 2 (cons 3 null)))", "(quote (1 2 3))");
    retval = retval | test_eval("(cons (1 2) 3)", "(quote ((1 2) . 3))");
    retval = retval | test_eval("(define x 0)", "(quote 0)");
    retval = retval | test_eval("x", "(quote 0)");
    retval = retval | test_eval("(+ x x)", "(quote (+ 0 0))");
    retval = retval | test_eval("(define y (quote (1 2 3)))", "(quote (1 2 3))");
    retval = retval | test_eval("y", "(quote (1 2 3))");
    retval = retval | test_eval("(cons x y)", "(quote (0 1 2 3))");
    retval = retval | test_eval("((lambda x x))", "(quote ())");
    retval = retval | test_eval("((lambda x x) 7)", "(quote (7))");
    retval = retval | test_eval("((lambda (z) z) 3)", "(quote 3)");
    retval = retval | test_eval("((lambda (z) (quote (1 2 3))) 2)", "(quote (1 2 3))");
    retval = retval | test_eval("((lambda z z) 5)", "(quote (5))");
    retval = retval | test_eval("(((lambda (y) (lambda (x) (cons x (cons y (quote()))))) 2) 3)", "(quote (3 2))");
    retval = retval | test_eval("(define f (lambda (x) x))", "#<procedure>");
    retval = retval | test_eval("(f 3)", "(quote 3)");
    retval = retval | test_eval("(define u (lambda () v))", "#<procedure>");
    retval = retval | test_eval("(second (quote (1 2 3 4)))", "(quote 2)");
    retval = retval | test_eval("(third (quote (1 2 3 4)))", "(quote 3)");
    retval = retval | test_eval("(define v 8)", "(quote 8)");
    retval = retval | test_eval("(u)", "(quote 8)");
    retval = retval | test_eval("((lambda (x y) (cons x (cons y null))) 1 2)", "(quote (1 2))");
    retval = retval | test_eval("((lambda x x) 1 2)", "(quote (1 2))");
    retval = retval | test_eval("(list 2 3 5 7)", "(quote (2 3 5 7))");
    retval = retval | test_eval("(list null null)", "(quote (() ()))");
    retval = retval | test_eval("(#t 1 0)", "(quote 1)");
    retval = retval | test_eval("(#f 1 0)", "(quote 0)");
    retval = retval | test_eval("(#t 5 (unknown))", "(quote 5)");
    retval = retval | test_eval("(#f (unknown) 4)", "(quote 4)");
    retval = retval | test_eval("((eq? 2 2) 1 0)", "(quote 1)");
    retval = retval | test_eval("((eq? 2 3) 1 0)", "(quote 0)");
    retval = retval | test_eval("((not #t) 1 0)", "(quote 0)");
    retval = retval | test_eval("((not #f) 1 0)", "(quote 1)");
    retval = retval | test_eval("((null? null) 1 0)", "(quote 1)");
    retval = retval | test_eval("((null? (quote ())) 1 0)", "(quote 1)");
    retval = retval | test_eval("((null? 2) 1 0)", "(quote 0)");
    retval = retval | test_eval("((null? (quote (1 2 3))) 1 0)", "(quote 0)");
    retval = retval | test_eval("((pair? (quote (1 2 3))) 1 0)", "(quote 1)");
    retval = retval | test_eval("((pair? (quote 1)) 1 0)", "(quote 0)");
    retval = retval | test_eval("((pair? null) 1 0)", "(quote 0)");
    retval = retval | test_eval("((and #f #f) 5 2)", "(quote 2)");
    retval = retval | test_eval("((and #f #t) 5 2)", "(quote 2)");
    retval = retval | test_eval("((and #t #f) 5 2)", "(quote 2)");
    retval = retval | test_eval("((and #t #t) 5 2)", "(quote 5)");
    retval = retval | test_eval("((or #f #f) 8 7)", "(quote 7)");
    retval = retval | test_eval("((or #f #t) 8 7)", "(quote 8)");
    retval = retval | test_eval("((or #t #f) 8 7)", "(quote 8)");
    retval = retval | test_eval("((or #t #t) 8 7)", "(quote 8)");
    retval = retval | test_eval("(if #t 6 3)", "(quote 6)");
    retval = retval | test_eval("(if #f 6 3)", "(quote 3)");
    retval = retval | test_eval("(define test (lambda (l) ((null? l) 0 (test (rest l)))))", "#<procedure>");
    retval = retval | test_eval("(test (quote (2 1 3)))", "(quote 0)");
    retval = retval | test_eval("(define member? (lambda (x l) (if (null? l) #f (if (eq? x (first l)) #t (member? x (rest l))))))", "#<procedure>");
    retval = retval | test_eval("((member? 1 (quote (3 1 2))) 1 0)", "(quote 1)");
    retval = retval | test_eval("((member? 1 (quote (3 4 2))) 1 0)", "(quote 0)");
    retval = retval | test_eval("(append (quote (1 2 3)) (quote (4 5)))", "(quote (1 2 3 4 5))");
    retval = retval | test_eval("(assoc 3 (quote ((1 a) (2 b) (3 c) (4 d))))", "(quote (3 c))");
    retval = retval | test_eval("(assoc 5 (quote ((1 a) (2 b) (3 c) (4 d))))", "#<procedure>");
    retval = retval | test_eval("(cond (#t 2) (#t 3))", "(quote 2)");
    retval = retval | test_eval("(cond (#f 2) (#t 3))", "(quote 3)");
    retval = retval | test_eval("(cond (#f 2) (#f 3))", "(quote ())");
    retval = retval | test_eval("((equal? 0 0) 1 0)", "(quote 1)");
    retval = retval | test_eval("((equal? 1 0) 1 0)", "(quote 0)");
    retval = retval | test_eval("((equal? (list 1) null) 1 0)", "(quote 0)");
    retval = retval | test_eval("((equal? (list 1 2) (list 1 2)) 1 0)", "(quote 1)");
    retval = retval | test_eval("((equal? (list 1 2) (list 1 2 3)) 1 0)", "(quote 0)");
    retval = retval | test_eval("((equal? (list 1 2) (list 1 1)) 1 0)", "(quote 0)");
  };
#else
  fprintf(stderr, "Cannot run tests without 'fmemopen'!\n");
#endif
  return retval;
}

