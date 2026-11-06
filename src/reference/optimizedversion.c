/*****************************************************
 *
 * optimizedversion.c : Resuelve puzles criptoaritmeticos
 *                      usando asignacion de letras y
 *                      verificacion recursiva por columnas
 *
 * Programmer: Elias
 *
 * Santiago, 04/11/2026
 *
 ****************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BASE 10
#define PUZ_SIZE 16

struct puzzle {
  char *words[PUZ_SIZE];
  unsigned char num_words;

  unsigned char assigned_digits[BASE];
  unsigned char used_digits[BASE];

  char unique_chars[BASE];
  unsigned char num_unique;
};

int parse(char *p, char *buff[]) {
  int len, in_word;

  len = 0;
  in_word = 0;
  while (*p) {
    if (isspace(*p) || *p == '+' || *p == '=') {
      *p = '\0';
      p = p + 1;
      in_word = 0;
    } else if (isalnum(*p)) {
      if (!in_word) {
        buff[len] = p;
        len = len + 1;
        in_word = 1;
      }
      p = p + 1;
    } else {
      return 0;
    }
  }
  buff[len] = NULL;
  return len;
}

int get_unique_chars(struct puzzle *puz) {
  unsigned int i, j, k;
  char ch;

  puz->num_unique = 0;
  for (i = 0; i < puz->num_words; i = i + 1) {
    for (j = 0; puz->words[i][j] != '\0'; j = j + 1) {
      ch = puz->words[i][j];
      k = 0;
      while (k < puz->num_unique && puz->unique_chars[k] != ch) {
        k = k + 1;
      }
      if (k == puz->num_unique) {
        if (puz->num_unique >= BASE) {
          return -1;
        }
        puz->unique_chars[puz->num_unique] = ch;
        puz->num_unique = puz->num_unique + 1;
      }
    }
  }
  return 0;
}

unsigned char char_to_digit(char c, struct puzzle *puz) {
  unsigned int i;

  for (i = 0; i < puz->num_unique; i = i + 1) {
    if (puz->unique_chars[i] == c) {
      return puz->assigned_digits[i];
    }
  }
  return 0;
}

// Verificacion recursiva columna a columna (solvePuzzleHelper de optimizedversion.cpp)
int solve_puzzle_helper(struct puzzle *puz, unsigned int pos, unsigned long carry) {
  unsigned long sum_val;
  unsigned int i, len, sum_len;
  char sum_char;

  sum_len = strlen(puz->words[puz->num_words - 1]);
  if (pos >= sum_len) {
    if (carry == 0) {
      return 1;
    }
    return 0;
  }

  sum_val = carry;
  for (i = 0; i < puz->num_words - 1; i = i + 1) {
    len = strlen(puz->words[i]);
    if (pos < len) {
      sum_val = sum_val + char_to_digit(puz->words[i][len - 1 - pos], puz);
    }
  }

  sum_char = puz->words[puz->num_words - 1][sum_len - 1 - pos];
  if (char_to_digit(sum_char, puz) != (sum_val % 10)) {
    return 0;
  }

  return solve_puzzle_helper(puz, pos + 1, sum_val / 10);
}

// Devuelve cuantas soluciones encontro (assignDigits de optimizedversion.cpp)
unsigned long search(struct puzzle *puz, unsigned char index, int print,
                     int first) {
  unsigned long count;
  unsigned int d, i;

  // Caso base: todas las letras tienen un digito asignado
  if (index == puz->num_unique) {
    // Una palabra de mas de una letra no puede empezar con 0
    for (i = 0; i < puz->num_words; i = i + 1) {
      if (puz->words[i][1] != '\0' &&
          char_to_digit(puz->words[i][0], puz) == 0) {
        return 0;
      }
    }
    // Verificacion recursiva columna a columna
    if (!solve_puzzle_helper(puz, 0, 0)) {
      return 0;
    }
    // Es solucion
    if (print) {
      for (i = 0; i < puz->num_unique; i = i + 1) {
        printf("%c=%d ", puz->unique_chars[i], puz->assigned_digits[i]);
      }
      printf("\n");
    }
    return 1;
  }
  count = 0;
  for (d = 0; d < 10; d = d + 1) {
    if (!puz->used_digits[d]) {
      puz->assigned_digits[index] = d;
      puz->used_digits[d] = 1;
      count = count + search(puz, index + 1, print, first);
      puz->used_digits[d] = 0;
      if (first && count > 0) {
        return count;
      }
    }
  }
  return count;
}

void bench(struct puzzle *puz, int print, int first) {
  unsigned long count;
  long t_start, t_end;
  float elapsed, sec, us;

  t_start = clock();
  count = search(puz, 0, 0, first);
  t_end = clock();
  elapsed = t_end - t_start;
  sec = elapsed / CLOCKS_PER_SEC;
  us = sec * 1000000.0;
  if (print) {
    search(puz, 0, 1, first);
  }
  printf("%lu solutions\n", count);
  printf("%f s (%.2f us)\n", sec, us);
}

// ./optimizedversion "PUZZLE+...+PIECES=SUM" <first, all, count>
int main(int argc, char *argv[]) {
  struct puzzle *puz;
  int print, first;

  puz = calloc(1, sizeof(struct puzzle));
  if (argc < 3) {
    printf("Usage: %s <PUZZLE> <first, all, count>\n", argv[0]);
    return 1;
  }
  puz->num_words = parse(argv[1], puz->words);
  if (puz->num_words < 2 || get_unique_chars(puz) < 0) {
    printf("error: puzle invalido\n");
    return 1;
  }
  print = strcmp(argv[2], "count") != 0;
  first = strcmp(argv[2], "first") == 0;
  bench(puz, print, first);
  free(puz);
  return 0;
}
