/*****************************************************
 *
 * 03_main.c : Resuelve puzles criptoaritmeticos
 *             usando asignacion de letras y recursividad
 *
 * Programmer:
 *
 * Santiago, 00/11/2026
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
  unsigned char word_len[PUZ_SIZE];
  unsigned char num_words;
  unsigned char assigned_digits[BASE];
  unsigned char used_digits[BASE];
  char unique_chars[BASE];
  unsigned char num_unique;
}; // 176 bytes

/*
 *
 */
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

/*
 *
 */
void get_unique_chars(struct puzzle *puz) {
  unsigned int i, j, k, m, len;
  char ch;

  m = 0;
  for (i = 0; i < puz->num_words; i = i + 1) {
    puz->word_len[i] = strlen(puz->words[i]);
    if (m < puz->word_len[i])
      m = puz->word_len[i];
  }
  puz->num_unique = 0;
  for (j = 0; j < m; j = j + 1) {
    for (i = 0; i < puz->num_words; i = i + 1) {
      len = puz->word_len[i];
      if (j < len) {
        ch = puz->words[i][len - 1 - j];
        k = 0;
        while (k < puz->num_unique && puz->unique_chars[k] != ch) {
          k = k + 1;
        }
        if (k == puz->num_unique) {
          if (puz->num_unique >= BASE)
            return;
          puz->unique_chars[puz->num_unique] = ch;
          puz->num_unique = puz->num_unique + 1;
        }
      }
    }
  }
}

/*
 *
 */
void index_words(struct puzzle *p) {
  int i, j, k;
  char ch;

  for (i = 0; i < p->num_words; i = i + 1) {
    for (j = 0; j < p->word_len[i]; j = j + 1) {
      ch = p->words[i][j];
      for (k = 0; k < p->num_unique; k = k + 1) {
        if (p->unique_chars[k] == ch)
          p->words[i][j] = k;
      }
    }
  }
}

/*
 * Verificacion recursiva columna a columna (como solvePuzzleHelper)
 */
int solve_puzzle_helper(struct puzzle *puz, unsigned int j, unsigned long carry,
                        unsigned char index) {
  unsigned long sum;
  unsigned int i, len, res_len, has_char;
  char res_char;
  unsigned char ch_idx;

  res_len = puz->word_len[puz->num_words - 1];
  sum = carry;
  has_char = 0;
  for (i = 0; i < puz->num_words - 1; i = i + 1) {
    len = puz->word_len[i];
    if (j < len) {
      has_char = 1;
      ch_idx = puz->words[i][len - 1 - j];
      if (ch_idx >= index)
        return 1;
      sum = sum + puz->assigned_digits[ch_idx];
    }
  }
  if (has_char == 0 && j >= res_len)
    return (carry == 0);
  if (j < res_len) {
    res_char = puz->words[puz->num_words - 1][res_len - 1 - j];
    if (res_char >= index)
      return 1;
    if (puz->assigned_digits[(int)res_char] != (sum % 10))
      return 0;
  } else {
    if (sum % 10 != 0)
      return 0;
  }
  return solve_puzzle_helper(puz, j + 1, sum / 10, index);
}

/*
 *  Devuelve cuantas soluciones encontro (equivalente a assignDigits)
 */
unsigned long search(struct puzzle *puz, unsigned char index, int print) {
  unsigned long count;
  unsigned int d, i;

  if (!solve_puzzle_helper(puz, 0, 0, index))
    return 0;
  // Caso base: todas las letras tienen un digito asignado
  if (index == puz->num_unique) {
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
      count = count + search(puz, index + 1, print);
      puz->used_digits[d] = 0;
    }
  }
  return count;
}

/*
 *
 */
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

/*
 *
 */
int main(int argc, char *argv[]) {
  struct puzzle *puz;
  int print;
  FILE *file;
  char line[256];

  if (argc == 3) {
    file = fopen(argv[1], "r");
    if (!file)
      return 1;
    while (fgets(line, sizeof(line), file)) {
      printf("\n%s", line);
      puz = calloc(1, sizeof(struct puzzle));
      puz->num_words = parse(line, puz->words);
      get_unique_chars(puz);
      index_words(puz);
      print = strcmp(argv[2], "count") != 0;
      bench(puz, print);
      free(puz);
    }
    fclose(file);
  } else
    printf("Usage: %s <filename> <all, count>\n", argv[0]);
  return 0;
}
