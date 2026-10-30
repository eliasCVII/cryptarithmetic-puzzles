// TODO: remove solutions[] and only print when solutions are found
/*****************************************************
 *
 * c_solver.c : Resuelve y mide puzles criptoaritmeticos
 *
 * Programmer:
 *
 * Santiago, 26/10/2026
 *
 ****************************************************/

#include "solver.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void bench_puzzle(char *words[], unsigned char len, int print, int first) {
  unsigned long count;
  long t_start, t_end;
  float sec, us;

  t_start = clock();
  count = solve(words, len, 0, first);
  t_end = clock();
  sec = (float)(t_end - t_start) / (float)CLOCKS_PER_SEC;
  us = sec * 1000000.0;
  if (print) {
    solve(words, len, 1, first);
  }
  printf("%lu solutions\n", count);
  printf("%f s (%.2f us)\n", sec, us);
}

// ./solver "PUZZLE+...+PIECES=SUM" <first, all, count>
//   argv[0] argv[1]                 argv[2]
int main(int argc, char *argv[]) {
  char *pp, *words[PUZ_SIZE];
  int len, print, first;

  if (argc < 3) {
    printf("Usage: %s <PUZZLE> <first, all, count>\n", argv[0]);
    return 1;
  }
  pp = argv[1];
  len = parse(pp, words); // guarda las palabras en *words[]
  print = strcmp(argv[2], "count") != 0;
  first = strcmp(argv[2], "first") == 0;
  bench_puzzle(words, len, print, first);
  return 0;
}
