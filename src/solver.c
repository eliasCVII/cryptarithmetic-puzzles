// TODO: usar aritmetica con punteroos
// TODO: Quitar arreglo de solutions y hacer print cada vez que se encuentra 1,
//       pero mantener un contador de soluciones

/*****************************************************
 *
 * solver.c : resuelve puzles Criptoaritmeticos?
 *
 * Programmer:
 *
 * Santiago, 26/10/2026
 *
 ****************************************************/

#include "solver.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int parse(char *p, char *buff[]) {
  int len, in_word;

  len = 0;
  in_word = 0;
  while (*p) {
    if (isspace(*p) || *p == '+' || *p == '=') {
      *p = '\0';
      p++;
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

int find_index(char c, char *unique_ch, unsigned char len) {
  int i;

  for (i = 0; i < len; i = i + 1) {
    if (c == (unique_ch[i]))
      return i;
  }
  return -1;
}

unsigned char get_unique_chars(char *words[PUZ_SIZE], unsigned char words_len,
                               char unique[]) {
  int i, j;
  unsigned char len, ch;

  len = 0;
  for (i = 0; i < words_len; i = i + 1) {
    for (j = 0; words[i][j] != '\0'; j = j + 1) {
      ch = words[i][j];
      if (find_index(ch, unique, len) == -1) {
        if (len >= BASE)
          return BASE + 1; // hay mas de 10 letras distintas
        unique[len] = ch;
        len = len + 1;
      }
    }
  }
  return len;
}

void compute_weights(char *words[PUZ_SIZE], unsigned char words_len,
                     char unique[], unsigned char unique_len, long weights[]) {
  int i, j, index, len;
  long m;
  char ch;

  for (i = 0; i < unique_len; i = i + 1) {
    weights[i] = 0;
  }
  for (i = 0; i < words_len; i = i + 1) {
    if (i < words_len - 1) {
      m = 1;
    } else {
      m = -1;
    }
    len = 0;
    while (words[i][len] != '\0') {
      len = len + 1;
    }
    for (j = len - 1; j >= 0; j = j - 1) {
      ch = words[i][j];
      index = find_index(ch, unique, unique_len);
      weights[index] = weights[index] + m;
      m = m * BASE;
    }
  }
}

void sort_chars(char unique[], long weights[], unsigned char unique_len) {
  int i, j, tope;
  long temp_w;
  char temp_c;

  for (i = 0; i < unique_len; i = i + 1) {
    tope = i;
    for (j = i + 1; j < unique_len; j = j + 1) {
      if (labs(weights[j]) > labs(weights[tope])) {
        tope = j;
      }
    }
    if (tope != i) {
      temp_w = weights[i];
      weights[i] = weights[tope];
      weights[tope] = temp_w;
      temp_c = unique[i];
      unique[i] = unique[tope];
      unique[tope] = temp_c;
    }
  }
}


void compute_rem_bounds(unsigned char unique_len, long weights[],
                        long max_rem[], long min_rem[]) {
  long pos_sum, neg_sum, w;
  int i;

  pos_sum = 0;
  neg_sum = 0;
  for (i = unique_len - 1; i >= 0; i = i - 1) {
    w = weights[i];
    if (w > 0) {
      pos_sum = pos_sum + (w * 9);
    } else if (w < 0) {
      neg_sum = neg_sum + (w * 9);
    }
    max_rem[i] = pos_sum;
    min_rem[i] = neg_sum;
  }
}

int find_next_digit(int index, long sum, int start_digit, long w, long max_rem,
                    long min_rem, int last_nz, unsigned char used[],
                    unsigned char domains[BASE][BASE]) {
  int d;
  long d_long;

  if (start_digit == 0 && (sum + max_rem < 0 || sum + min_rem > 0)) {
    return -1;
  }
  if (index == last_nz && w != 0) {
    if (start_digit == 0 && (-sum) % w == 0) {
      d_long = (-sum) / w;
      if (d_long >= 0 && d_long < BASE) {
        d = d_long;
        if (used[d] == 0 && domains[index][d] == 1) {
          return d;
        }
      }
    }
    return -1;
  }
  for (d = start_digit; d < BASE; d = d + 1) {
    if (used[d] == 0 && domains[index][d] == 1) {
      return d;
    }
  }
  return -1;
}

void presolve(char *words[PUZ_SIZE], unsigned char words_len, char unique[],
              unsigned char unique_len, unsigned char domains[BASE][BASE]) {
  int i, j, letter_pos, target_ch, fixed_digit, one_count, len, max_len;

  // Todos los digitos (0-9) son una posible asignacion
  for (i = 0; i < unique_len; i = i + 1) {
    for (j = 0; j < BASE; j = j + 1) {
      domains[i][j] = 1;
    }
  }
  // Quitar 0 como valor para word[0] si strlen(word) > 0
  for (i = 0; i < words_len; i = i + 1) {
    if (words[i][1] != '\0') {
      letter_pos = find_index(words[i][0], unique, unique_len);
      domains[letter_pos][0] = 0;
    }
  }
  max_len = 0;
  for (i = 0; i < words_len - 1; i = i + 1) {
    j = 0;
    while (words[i][j] != '\0') {
      j = j + 1;
    }
    if (j > max_len) {
      max_len = j;
    }
  }
  len = 0;
  while (words[words_len - 1][len] != '\0') {
    len = len + 1;
  }
  // Acarreo entre dos letras -> la resultante es 1 o 0
  if (words_len - 1 == 2 && len > max_len) {
    target_ch = find_index(words[words_len - 1][0], unique, unique_len);
    for (j = 0; j < BASE; j = j + 1) {
      domains[target_ch][j] = 0;
    }
    domains[target_ch][1] = 1;
  }
  for (i = 0; i < unique_len; i = i + 1) {
    one_count = 0;
    fixed_digit = -1;
    for (j = 0; j < BASE; j = j + 1) {
      if (domains[i][j] == 1) {
        one_count = one_count + 1;
        fixed_digit = j;
      }
    }
    if (one_count == 1) {
      for (j = 0; j < unique_len; j = j + 1) {
        if (j != i) {
          domains[j][fixed_digit] = 0;
        }
      }
    }
  }
}

unsigned long solve(char *words[PUZ_SIZE], unsigned char len, int print,
                    int first) {
  int i, d, index, last_nz, next_digit[BASE + 1];
  unsigned long count;
  char unique_ch[BASE];
  long sum_stack[BASE + 1];
  unsigned char unique_len, domains[BASE][BASE], assigned[BASE], used[BASE];
  long weights[BASE], max_rem[BASE], min_rem[BASE];

  for (i = 0; i < BASE; i = i + 1) {
    used[i] = 0;
    assigned[i] = 0;
  }
  unique_len = get_unique_chars(words, len, unique_ch);
  if (unique_len > BASE || unique_len == 0) {
    return 0;
  }
  compute_weights(words, len, unique_ch, unique_len, weights);
  sort_chars(unique_ch, weights, unique_len);
  last_nz = 0;
  for (i = 0; i < unique_len; i = i + 1) {
    if (weights[i] != 0) {
      last_nz = i;
    }
  }
  presolve(words, len, unique_ch, unique_len, domains);
  compute_rem_bounds(unique_len, weights, max_rem, min_rem);
  count = 0;
  index = 0;
  sum_stack[0] = 0;
  next_digit[0] = 0;
  while (index >= 0) {
    // Estamos en una solucion
    if (index == unique_len) {
      if (sum_stack[index] == 0) {
        count = count + 1;
        if (print) {
          for (i = 0; i < unique_len; i = i + 1) {
            printf("%c=%d ", unique_ch[i], assigned[i]);
          }
          printf("\n");
        }
        if (first) {
          break;
        }
      }
      index = index - 1;
      if (index >= 0) {
        used[assigned[index]] = 0;
      }
    } else {
      d = find_next_digit(index, sum_stack[index], next_digit[index],
                          weights[index], max_rem[index], min_rem[index],
                          last_nz, used, domains);
      if (d != -1) {
        assigned[index] = d;
        used[d] = 1;
        sum_stack[index + 1] = sum_stack[index] + (weights[index] * d);
        next_digit[index] = d + 1;
        index = index + 1;
        next_digit[index] = 0;
      } else {
        index = index - 1;
        if (index >= 0) {
          used[assigned[index]] = 0;
        }
      }
    }
  }
  return count;
}
