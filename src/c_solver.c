/*****************************************************
 *
 * solver.c : resuelve puzles Criptoaritmeticos?
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

int main(void) {
  char p1[3][MAX_WORD_LEN] = {"SEND", "MORE", "MONEY"};
  char p2[4][MAX_WORD_LEN] = {"CP", "IS", "FUN", "TRUE"};
  char p3[3][MAX_WORD_LEN] = {"SO", "MANY", "MORE"};
  char p4[4][MAX_WORD_LEN] = {"AB", "CD", "EF", "GHI"};
  char unique[BASE];
  unsigned char unique_len;
  unsigned char solutions[SOLUTIONS_SIZE * BASE];
  unsigned long count;

  printf("=== CRYPTARITHMETIC PUZZLE SOLVER (FLAT C) ===\n\n");

  printf(">> SEND + MORE = MONEY:\n");
  count = solve(p1, 3, unique, &unique_len, solutions, SOLUTIONS_SIZE);
  print_solutions(p1, 3, unique, unique_len, solutions, count, 5);

  printf("\n>> CP + IS + FUN = TRUE:\n");
  count = solve(p2, 4, unique, &unique_len, solutions, SOLUTIONS_SIZE);
  print_solutions(p2, 4, unique, unique_len, solutions, count, 3);

  printf("\n>> SO + MANY = MORE:\n");
  count = solve(p3, 3, unique, &unique_len, solutions, SOLUTIONS_SIZE);
  print_solutions(p3, 3, unique, unique_len, solutions, count, 3);

  printf("\n>> AB + CD + EF = GHI:\n");
  count = solve(p4, 4, unique, &unique_len, solutions, SOLUTIONS_SIZE);
  print_solutions(p4, 4, unique, unique_len, solutions, count, 3);

  return 0;
}
