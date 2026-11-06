/*****************************************************
 *
 * recursiva.c : Resuelve y mide puzles criptoaritmeticos
 *                       por backtracking recursivo
 *
 * Programmer:
 *
 * Santiago, 01/11/2026
 *
 ****************************************************/

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define BASE 10
#define PUZ_SIZE 16

struct puzzle {
  char *words[PUZ_SIZE]; // apuntan dentro de argv[1]
  unsigned char num_words;

  unsigned char assigned_digits[BASE]; // digito de unique_chars[i]
  unsigned char used_digits[BASE];     // 1 si el digito ya esta asignado

  char unique_chars[BASE]; // letras distintas
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
      continue;
    }
    if (isalnum(*p)) {
      if (!in_word) {
        buff[len] = p;
        len = len + 1;
        in_word = 1;
      }
      p = p + 1;
      continue;
    }
    return 0;
  }
  buff[len] = NULL;
  return len;
}

// Devuelve -1 si hay mas de BASE letras distintas
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

unsigned long word_to_number(char word[], struct puzzle *puz) {
  unsigned long result;
  unsigned int i;

  result = 0;
  for (i = 0; word[i] != '\0'; i = i + 1) {
    result = result * 10 + char_to_digit(word[i], puz);
  }
  return result;
}

// Devuelve cuantas soluciones encontro
unsigned long search(struct puzzle *puz, unsigned char index, int print) {
  unsigned long count, sum;
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
    // La suma de los sumandos debe ser igual a la ultima palabra
    sum = 0;
    for (i = 0; i < puz->num_words - 1; i = i + 1) {
      sum = sum + word_to_number(puz->words[i], puz);
    }
    if (sum != word_to_number(puz->words[puz->num_words - 1], puz)) {
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

  // Recursividad
  count = 0;
  for (d = 0; d < 10; d = d + 1) {
    if (!puz->used_digits[d]) {
      puz->assigned_digits[index] = d;
      puz->used_digits[d] = 1;
      count = count + search(puz, index + 1, print);
      puz->used_digits[d] = 0;
    }
  }
  return count;
}

void bench(struct puzzle *puz, int print) {
  unsigned long count;
  long t_start, t_end;
  float elapsed, sec, us;

  t_start = clock();
  count = search(puz, 0, 0);
  t_end = clock();
  elapsed = t_end - t_start;
  sec = elapsed / CLOCKS_PER_SEC;
  us = sec * 1000000.0;
  if (print)
    search(puz, 0, 1);
  printf("%lu solutions\n", count);
  printf("%f s (%.2f us)\n", sec, us);
}

int main(int argc, char *argv[]) {
  struct puzzle *puzzle;
  int print;

  puzzle = calloc(1, sizeof(struct puzzle));
  if (argc < 3) {
    printf("Usage: %s <PUZZLE> <first, all, count>\n", argv[0]);
    return 1;
  }
  puzzle->num_words = parse(argv[1], puzzle->words);
  if (puzzle->num_words < 2 || get_unique_chars(puzzle) < 0) {
    printf("error: puzle invalido\n");
    return 1;
  }
  print = strcmp(argv[2], "count") != 0;
  bench(puzzle, print);
  free(puzzle);
  return 0;
}
