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
#include <assert.h>
#include <string.h>
#include "blc.h"
#include "lambda.h"

#define BUFSIZE 256

int test_compile(char *cmd, char *spec)
{
  int retval = 0;
  char buffer[BUFSIZE];
  char *result = buffer;
  FILE *f_in = fmemopen(cmd, strlen(cmd), "r");
  FILE *f_out = fmemopen(result, BUFSIZE, "w");
  compile_lambda(f_in, f_out);
  fclose(f_out);
  fclose(f_in);
  if (strcmp(spec, result)) {
    fprintf(stderr, "Result of compiling \"%s\" is \"%s\" but should be \"%s\"\n", cmd, result, spec);
    retval = 1;
  };
  return retval;
}

int main(void)
{
  int retval = 0;
  retval = retval | test_compile("0010", "0010");
  retval = retval | test_compile("00.10", "0010");
  retval = retval | test_compile("-10", "10");
  retval = retval | test_compile("-10>", "10");
  retval = retval | test_compile("->x.x", "0010");
  retval = retval | test_compile("->xy.xy", "0010");
  retval = retval | test_compile("-> x.x", "0010");
  retval = retval | test_compile("->x .x", "0010");
  retval = retval | test_compile("->x. x", "0010");
  retval = retval | test_compile("->x.->y.x", "0000110");
  retval = retval | test_compile("->x.->y.y", "000010");
  retval = retval | test_compile("->x.->x.x", "000010");
  retval = retval | test_compile("00->x.x", "000010");
  retval = retval | test_compile("->.->y.y", "000010");
  retval = retval | test_compile("->->y.y", "000010");
  retval = retval | test_compile("->x->y.x", "0000110");
  retval = retval | test_compile("->x y.x", "0000110");
  retval = retval | test_compile("->x.->y.x", "0000110");
  retval = retval | test_compile("->x->y.x", "0000110");
  retval = retval | test_compile("->x->.x", "0000110");
  retval = retval | test_compile("-> x -> y . x", "0000110");
  retval = retval | test_compile("011->->x.x->y.y", "0110000100010");
  retval = retval | test_compile("011000010->y.y", "0110000100010");
  retval = retval | test_compile("011->x.x 000010", "0110010000010");
  retval = retval | test_compile("011 ->->x.x ->y.y", "0110000100010");
  retval = retval | test_compile("011 ->->x.x ->x.x", "0110000100010");
  retval = retval | test_compile("f=->->x.x f", "01000001010");
  retval = retval | test_compile("f= ->->x.x f", "01000001010");
  retval = retval | test_compile("f =->->x.x f", "01000001010");
  retval = retval | test_compile("f=->->x.x f f", "0100000101010");
  retval = retval | test_compile("->x.y=x y", "000101010");
  retval = retval | test_compile("(->->x.x ->y.y)", "0110000100010");
  retval = retval | test_compile("(->->x.x ->x.x)", "0110000100010");
  retval = retval | test_compile("((->x->y.x input) output)", "011011000011010110");
  retval = retval | test_compile("((->x->y.y input) output)", "01101100001010110");
  retval = retval | test_compile("(->x->y.x input output)", "011011000011010110");
  retval = retval | test_compile("(->x->y.y input output)", "01101100001010110");
  retval = retval | test_compile("(input ->x->y.x) 0", "0111000001100");
  retval = retval | test_compile("(input)", "10");
  retval = retval | test_compile("((((((->input->output->I->true->false->Y."
                                 "((Y->f->input.(((input true)true)(f(input false))))input)"
                                 "input)output)->x.x)->x->y.x)->x->y.y)->f.(->x.(f(x x))->x.(f(x x))))",
                                 "01101101101101101100000000000001101110000001"
                                 "10110111011111011111001111001110111101111110"
                                 "10110001000001100000100001100011110011101000"
                                 "0111100111010");
  return retval;
}

