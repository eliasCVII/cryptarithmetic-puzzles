/*****************************************************
 *
 * initial_recursion.c : Resuelve y mide puzles criptoaritmeticos
 *                       por backtracking recursivo
 *
 * Programmer:
 *
 * Santiago, 01/11/2025
 *
 ****************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_UNIQUE 10
#define NUM_CHARS 128 // parse() solo deja pasar caracteres alfanumericos ASCII
#define PUZ_SIZE 16

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

int get_unique_chars(char *words[], unsigned int num_words,
                     char unique_chars[]) {
  int num_unique;
  unsigned int i, j;
  unsigned char ch;
  char seen[NUM_CHARS];

  num_unique = 0;
  for (i = 0; i < NUM_CHARS; i = i + 1) {
    seen[i] = 0;
  }

  for (i = 0; i < num_words; i = i + 1) {
    for (j = 0; words[i][j] != '\0'; j = j + 1) {
      ch = words[i][j];
      if (!seen[ch]) {
        seen[ch] = 1;
        if (num_unique >= MAX_UNIQUE) {
          return -1;
        }
        unique_chars[num_unique] = ch;
        num_unique = num_unique + 1;
      }
    }
  }
  return num_unique;
}

unsigned char char_to_digit(char c, char unique_chars[],
                            unsigned int num_unique,
                            unsigned char assigned_digits[]) {
  unsigned int i;

  for (i = 0; i < num_unique; i = i + 1) {
    if (unique_chars[i] == c) {
      return assigned_digits[i];
    }
  }
  return 0;
}

unsigned long word_to_number(char word[], char unique_chars[],
                             unsigned int num_unique,
                             unsigned char assigned_digits[]) {
  unsigned long result;
  unsigned int i;

  result = 0;
  for (i = 0; word[i] != '\0'; i = i + 1) {
    result = result * 10 +
             char_to_digit(word[i], unique_chars, num_unique, assigned_digits);
  }
  return result;
}

// Devuelve cuantas soluciones encontro (como maximo 1 si first)
unsigned long search(char *words[], unsigned int num_words, unsigned int index,
                     char unique_chars[], unsigned int num_unique,
                     unsigned char assigned_digits[], char used_digits[],
                     int print, int first) {
  unsigned long count, sum;
  unsigned int d, i;

  // Caso base: todas las letras tienen un digito asignado
  if (index == num_unique) {
    // Una palabra de mas de una letra no puede empezar con 0
    for (i = 0; i < num_words; i = i + 1) {
      if (words[i][1] != '\0' &&
          char_to_digit(words[i][0], unique_chars, num_unique,
                        assigned_digits) == 0) {
        return 0;
      }
    }
    // La suma de los sumandos debe ser igual a la ultima palabra
    sum = 0;
    for (i = 0; i < num_words - 1; i = i + 1) {
      sum = sum +
            word_to_number(words[i], unique_chars, num_unique, assigned_digits);
    }
    if (sum != word_to_number(words[num_words - 1], unique_chars, num_unique,
                              assigned_digits)) {
      return 0;
    }
    // Es solucion
    if (print) {
      for (i = 0; i < num_unique; i = i + 1) {
        printf("%c=%d ", unique_chars[i], assigned_digits[i]);
      }
      printf("\n");
    }
    return 1;
  }
  count = 0;
  for (d = 0; d < 10; d = d + 1) {
    if (!used_digits[d]) {
      assigned_digits[index] = d;
      used_digits[d] = 1;
      count =
          count + search(words, num_words, index + 1, unique_chars, num_unique,
                         assigned_digits, used_digits, print, first);
      used_digits[d] = 0;
      if (first && count > 0) {
        return count;
      }
    }
  }
  return count;
}

void bench(char *words[], unsigned int num_words, char unique_chars[],
                  unsigned int num_unique, int print, int first) {
  unsigned char assigned_digits[MAX_UNIQUE];
  char used_digits[10] = {0};
  unsigned long count;
  long t_start, t_end;
  float elapsed, sec, us;

  t_start = clock();
  count = search(words, num_words, 0, unique_chars, num_unique, assigned_digits,
                 used_digits, 0, first);
  t_end = clock();
  elapsed = t_end - t_start;
  sec = elapsed / CLOCKS_PER_SEC;
  us = sec * 1000000.0;
  if (print) {
    search(words, num_words, 0, unique_chars, num_unique, assigned_digits,
           used_digits, 1, first);
  }
  printf("%lu solutions\n", count);
  printf("%f s (%.2f us)\n", sec, us);
}

// ./initial_recursion "PUZZLE+...+PIECES=SUM" <first, all, count>
//   argv[0]           argv[1]                 argv[2]
int main(int argc, char *argv[]) {
  char *words[PUZ_SIZE];
  char unique_chars[MAX_UNIQUE];
  int num_words, num_unique, print, first;

  if (argc < 3) {
    printf("Usage: %s <PUZZLE> <first, all, count>\n", argv[0]);
    return 1;
  }
  num_words = parse(argv[1], words);
  if (num_words < 2) {
    printf("error: puzle invalido\n");
    return 1;
  }
  num_unique = get_unique_chars(words, num_words, unique_chars);
  if (num_unique < 0) {
    printf("error: demasiadas letras distintas\n");
    return 1;
  }
  print = strcmp(argv[2], "count") != 0;
  first = strcmp(argv[2], "first") == 0;
  bench(words, num_words, unique_chars, num_unique, print, first);
  return 0;
}
