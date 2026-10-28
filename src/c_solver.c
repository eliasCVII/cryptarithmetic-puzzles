/*****************************************************
 *
 * c_solver.c : Resuelve y mide puzles criptoaritmeticos
 *
 * Programmer:
 *
 * Santiago, 26/10/2026
 *
 ****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "solver.h"

void bench_puzzle(char words[][MAX_WORD_LEN], unsigned char num_words,
                  char name[], unsigned char solutions[],
                  unsigned long max_solutions, unsigned long max_to_print) {
  char unique[BASE];
  unsigned char unique_len;
  unsigned long count;
  long t_start, t_end;
  float sec, us;

  t_start = clock();
  count = solve(words, num_words, unique, &unique_len, solutions,
                max_solutions);
  t_end = clock();

  sec = (float)(t_end - t_start) / (float)CLOCKS_PER_SEC;
  us = sec * 1000000.0;

  printf(">> %s: %f s (%.2f us) [solutions: %lu]\n", name, sec, us, count);
  if (max_to_print > 0) {
    print_solutions(words, num_words, unique, unique_len, solutions, count,
                    max_to_print);
  }
  printf("\n");
}

int main(void) {
  unsigned long max_solutions;
  unsigned char *solutions;

  char p1[3][MAX_WORD_LEN] = {"SEND", "MORE", "MONEY"};
  char p2[3][MAX_WORD_LEN] = {"BASE", "BALL", "GAMES"};
  char p3[3][MAX_WORD_LEN] = {"CROSS", "ROADS", "DANGER"};
  char p4[4][MAX_WORD_LEN] = {"CP", "IS", "FUN", "TRUE"};
  char p5[3][MAX_WORD_LEN] = {"ABC", "DEF", "GHIJ"};
  char p6[3][MAX_WORD_LEN] = {"SO", "MANY", "MORE"};
  char p7[4][MAX_WORD_LEN] = {"AB", "CD", "EF", "GHI"};
  char p8[4][MAX_WORD_LEN] = {"AB", "CDE", "FG", "HIJ"};
  char p9[5][MAX_WORD_LEN] = {"AB", "CD", "EF", "G", "HIJ"};
  char p10[6][MAX_WORD_LEN] = {"A", "B", "C", "D", "E", "FG"};
  char p11[7][MAX_WORD_LEN] = {"A", "B", "C", "D", "E", "F", "GH"};
  char p12[8][MAX_WORD_LEN] = {"A", "B", "C", "D", "E", "F", "G", "HI"};
  char p13[9][MAX_WORD_LEN] = {"A", "B", "C", "D", "E", "F", "G", "H", "IJ"};

  max_solutions = 50000;
  solutions = calloc(max_solutions, BASE);
  if (solutions == 0) {
    printf("Error al asignar memoria en heap\n");
    return 1;
  }
  bench_puzzle(p1, 3, "SEND + MORE = MONEY", solutions, max_solutions, 1);
  bench_puzzle(p2, 3, "BASE + BALL = GAMES", solutions, max_solutions, 1);
  bench_puzzle(p3, 3, "CROSS + ROADS = DANGER", solutions, max_solutions, 1);
  bench_puzzle(p4, 4, "CP + IS + FUN = TRUE", solutions, max_solutions, 2);
  bench_puzzle(p5, 3, "ABC + DEF = GHIJ", solutions, max_solutions, 2);
  bench_puzzle(p6, 3, "SO + MANY = MORE", solutions, max_solutions, 2);
  bench_puzzle(p7, 4, "AB + CD + EF = GHI", solutions, max_solutions, 2);
  bench_puzzle(p8, 4, "AB + CDE + FG = HIJ", solutions, max_solutions, 2);
  bench_puzzle(p9, 5, "AB + CD + EF + G = HIJ", solutions, max_solutions, 2);
  bench_puzzle(p10, 6, "A + B + C + D + E = FG", solutions, max_solutions, 2);
  bench_puzzle(p11, 7, "A + B + C + D + E + F = GH", solutions, max_solutions, 2);
  bench_puzzle(p12, 8, "A + B + C + D + E + F + G = HI", solutions, max_solutions, 2);
  bench_puzzle(p13, 9, "A + B + C + D + E + F + G + H = IJ", solutions, max_solutions, 2);

  free(solutions);
  return 0;
}
