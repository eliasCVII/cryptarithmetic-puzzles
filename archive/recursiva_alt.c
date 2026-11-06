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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_UNIQUE 10
#define PUZ_SIZE 16

struct puzzle {
  char *words[PUZ_SIZE];         // apuntan dentro de argv[1]
  unsigned char w_len[PUZ_SIZE]; // largo de la palabra individual, 'SEND' = 4
  unsigned char words_len;

  unsigned char assigned_digits[MAX_UNIQUE]; // digito de unique_chars[i]
  unsigned char used_digits[10];             // 1 si el digito ya esta asignado

  char unique_chars[MAX_UNIQUE]; // letras distintas
  unsigned char unique_len;
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

// Devuelve -1 si hay mas de MAX_UNIQUE letras distintas
int get_unique_chars(struct puzzle *puz) {
  unsigned int i, j, k;
  char ch;

  puz->unique_len = 0;
  for (i = 0; i < puz->words_len; i = i + 1) {
    for (j = 0; puz->words[i][j] != '\0'; j = j + 1) {
      ch = puz->words[i][j];
      k = 0;
      while (k < puz->unique_len && puz->unique_chars[k] != ch) {
        k = k + 1;
      }
      if (k == puz->unique_len) {
        if (puz->unique_len >= MAX_UNIQUE) {
          return -1;
        }
        puz->unique_chars[puz->unique_len] = ch;
        puz->unique_len = puz->unique_len + 1;
      }
    }
  }
  return 0;
}

void index_words(struct puzzle *p) {
  int i, j, k;
  char ch;

  for (i = 0; i < p->words_len; i = i + 1) {
    p->w_len[i] = strlen(p->words[i]);
    for (j = 0; j < p->w_len[i]; j = j + 1) {
      ch = p->words[i][j];
      for (k = 0; k < p->unique_len; k = k + 1) {
        if (p->unique_chars[k] == ch) {
          p->words[i][j] = (char)k;
          break;
        }
      }
    }
  }
}

unsigned long word_to_number(struct puzzle *puz, unsigned int w_idx) {
  unsigned long result;
  unsigned int i;
  unsigned char ch_idx;

  result = 0;
  for (i = 0; i < puz->w_len[w_idx]; i = i + 1) {
    ch_idx = (unsigned char)puz->words[w_idx][i];
    result = result * 10 + puz->assigned_digits[ch_idx];
  }
  return result;
}

// Devuelve cuantas soluciones encontro (como maximo 1 si first==1)
unsigned long search(struct puzzle *puz, unsigned char index, int print,
                     int first) {
  unsigned long count, sum;
  unsigned int d, i;
  unsigned char ch_idx;

  // Caso base: todas las letras tienen un digito asignado
  if (index == puz->unique_len) {
    // Una palabra de mas de una letra no puede empezar con 0
    for (i = 0; i < puz->words_len; i = i + 1) {
      ch_idx = puz->words[i][0];
      if (puz->w_len[i] > 1 && puz->assigned_digits[ch_idx] == 0) {
        return 0;
      }
    }
    // La suma de los sumandos debe ser igual a la ultima palabra
    sum = 0;
    for (i = 0; i < puz->words_len - 1; i = i + 1) {
      sum = sum + word_to_number(puz, i);
    }
    if (sum != word_to_number(puz, puz->words_len - 1)) {
      return 0;
    }
    // Es solucion
    if (print) {
      for (i = 0; i < puz->unique_len; i = i + 1) {
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

// ./r "PUZZLE+...+PIECES=SUM" <first, all, count>
int main(int argc, char *argv[]) {
  struct puzzle *puz;
  int print, first;

  puz = calloc(1, sizeof(struct puzzle));
  if (argc < 3) {
    printf("Usage: %s <PUZZLE> <first, all, count>\n", argv[0]);
    return 1;
  }
  puz->words_len = parse(argv[1], puz->words);
  if (puz->words_len < 2 || get_unique_chars(puz) < 0) {
    printf("error: puzle invalido\n");
    return 1;
  }
  print = strcmp(argv[2], "count") != 0;
  first = strcmp(argv[2], "first") == 0;

  index_words(puz);
  bench(puz, print, first);
  free(puz);
  return 0;
}
