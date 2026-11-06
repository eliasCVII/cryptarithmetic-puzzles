/*****************************************************
 *
 * backtrackingO10n.c : Resuelve puzles criptoaritmeticos
 *                      usando backtracking sobre las 26
 *                      letras del abecedario
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

#define ALPHABET_SIZE 26
#define BASE 10
#define PUZ_SIZE 16

struct puzzle {
  char *words[PUZ_SIZE];
  unsigned char num_words;

  unsigned char present[ALPHABET_SIZE];
  unsigned char assigned_digits[ALPHABET_SIZE];
  unsigned char used_digits[BASE];

  unsigned char num_present;
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

int mark_present_chars(struct puzzle *puz) {
  unsigned int i, j;
  int idx;
  char ch;

  puz->num_present = 0;
  for (i = 0; i < ALPHABET_SIZE; i = i + 1) {
    puz->present[i] = 0;
    puz->assigned_digits[i] = 0;
  }
  for (i = 0; i < BASE; i = i + 1) {
    puz->used_digits[i] = 0;
  }
  for (i = 0; i < puz->num_words; i = i + 1) {
    for (j = 0; puz->words[i][j] != '\0'; j = j + 1) {
      ch = puz->words[i][j];
      if (ch >= 'a' && ch <= 'z') {
        idx = ch - 'a';
      } else if (ch >= 'A' && ch <= 'Z') {
        idx = ch - 'A';
      } else {
        return -1;
      }
      if (!puz->present[idx]) {
        if (puz->num_present >= BASE) {
          return -1;
        }
        puz->present[idx] = 1;
        puz->num_present = puz->num_present + 1;
      }
    }
  }
  return 0;
}

unsigned char char_to_digit(char c, struct puzzle *puz) {
  if (c >= 'a' && c <= 'z') {
    return puz->assigned_digits[c - 'a'];
  }
  if (c >= 'A' && c <= 'Z') {
    return puz->assigned_digits[c - 'A'];
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
unsigned long search(struct puzzle *puz, unsigned char ind, int print,
                     int first) {
  unsigned long count, sum;
  unsigned int d, i;

  // Caso base: se procesaron las 26 letras del abecedario
  if (ind == ALPHABET_SIZE) {
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
      for (i = 0; i < ALPHABET_SIZE; i = i + 1) {
        if (puz->present[i]) {
          printf("%c=%d ", 'A' + i, puz->assigned_digits[i]);
        }
      }
      printf("\n");
    }
    return 1;
  }

  // Si la letra no aparece en el puzle, se salta a la siguiente
  if (!puz->present[ind]) {
    return search(puz, ind + 1, print, first);
  }

  count = 0;
  for (d = 0; d < 10; d = d + 1) {
    if (!puz->used_digits[d]) {
      puz->assigned_digits[ind] = d;
      puz->used_digits[d] = 1;
      count = count + search(puz, ind + 1, print, first);
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

int main(int argc, char *argv[]) {
  struct puzzle *puz;
  int print, first;

  puz = calloc(1, sizeof(struct puzzle));
  if (argc < 3) {
    printf("Usage: %s <PUZZLE> <first, all, count>\n", argv[0]);
    return 1;
  }
  puz->num_words = parse(argv[1], puz->words);
  if (puz->num_words < 2 || mark_present_chars(puz) < 0) {
    printf("error: puzle invalido\n");
    return 1;
  }
  print = strcmp(argv[2], "count") != 0;
  first = strcmp(argv[2], "first") == 0;
  bench(puz, print, first);
  free(puz);
  return 0;
}
