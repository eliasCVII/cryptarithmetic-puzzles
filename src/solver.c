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

int find_index(char c, char unique[], unsigned char len) {
  int i;

  for (i = 0; i < len; i = i + 1) {
    if (c == unique[i])
      return i;
  }
  return -1;
}

unsigned char get_unique_chars(char words[][MAX_WORD_LEN],
                               unsigned char num_words, char unique[]) {
  int i, j;
  unsigned char len, ch;

  len = 0;
  for (i = 0; i < num_words; i = i + 1) {
    for (j = 0; words[i][j] != '\0'; j = j + 1) {
      ch = words[i][j];
      if (find_index(ch, unique, len) == -1) {
        if (len >= BASE)
          break;
        unique[len] = ch;
        len = len + 1;
      }
    }
  }
  return len;
}

void compute_weights(char words[][MAX_WORD_LEN], unsigned char num_words,
                     char unique[], unsigned char unique_len, long weights[]) {
  int i, j, index, len;
  long m;
  char ch;

  for (i = 0; i < unique_len; i = i + 1) {
    weights[i] = 0;
  }
  for (i = 0; i < num_words; i = i + 1) {
    if (i < num_words - 1) {
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

long word_to_number(char word[], char unique[], unsigned char unique_len,
                    unsigned char assigned_digits[]) {
  long result;
  int i, index;

  result = 0;
  for (i = 0; word[i] != '\0'; i = i + 1) {
    index = find_index(word[i], unique, unique_len);
    result = result * BASE + (long)assigned_digits[index];
  }
  return result;
}

void presolve(char words[][MAX_WORD_LEN], unsigned char num_words,
              char unique[], unsigned char unique_len,
              unsigned char domains[BASE][BASE]) {
  int i, j, letter_pos, target_ch, fixed_digit, one_count, len, max_len;

  // Todos los digitos (0-9) son una posible asignacion
  for (i = 0; i < unique_len; i = i + 1) {
    for (j = 0; j < BASE; j = j + 1) {
      domains[i][j] = 1;
    }
  }
  // Quitar 0 como valor para word[0] si strlen(word) > 0
  for (i = 0; i < num_words; i = i + 1) {
    if (words[i][1] != '\0') {
      letter_pos = find_index(words[i][0], unique, unique_len);
      domains[letter_pos][0] = 0;
    }
  }
  max_len = 0;
  for (i = 0; i < num_words - 1; i = i + 1) {
    j = 0;
    while (words[i][j] != '\0') {
      j = j + 1;
    }
    if (j > max_len) {
      max_len = j;
    }
  }
  len = 0;
  while (words[num_words - 1][len] != '\0') {
    len = len + 1;
  }
  // Acarreo entre dos letras -> la resultante es 1 o 0
  if (num_words - 1 == 2 && len > max_len) {
    target_ch = find_index(words[num_words - 1][0], unique, unique_len);
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

void precomp_branches(unsigned char unique_len, long weights[],
                      long max_suffix[], long min_suffix[]) {
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
    max_suffix[i] = pos_sum;
    min_suffix[i] = neg_sum;
  }
}

int find_next_digit(int index, long sum, int start_digit, long w,
                    long max_suffix, long min_suffix, int last_nz,
                    unsigned char used[], unsigned char domains[BASE][BASE]) {
  int d;
  long d_long;

  if (start_digit == 0 && (sum + max_suffix < 0 || sum + min_suffix > 0)) {
    return -1;
  }
  if (index == last_nz && w != 0) {
    if (start_digit == 0 && (-sum) % w == 0) {
      d_long = (-sum) / w;
      if (d_long >= 0 && d_long < BASE) {
        d = (int)d_long;
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

void save_solution(unsigned char solutions[], unsigned long max_solutions,
                   unsigned long sol_count, unsigned char assigned[],
                   unsigned char unique_len) {
  int i;
  unsigned long offset;

  if (sol_count < max_solutions) {
    offset = sol_count * BASE;
    for (i = 0; i < (int)unique_len; i = i + 1) {
      solutions[offset + i] = assigned[i];
    }
  }
}

unsigned long solve(char words[][MAX_WORD_LEN], unsigned char num_words,
                    char unique[], unsigned char *out_unique_len,
                    unsigned char solutions[], unsigned long max_solutions) {
  int i, d, index, last_nz, next_digit[BASE + 1];
  long sum_stack[BASE + 1];
  unsigned long sol_count;
  unsigned char unique_len, domains[BASE][BASE], assigned[BASE], used[BASE];
  long weights[BASE], max_suffix[BASE], min_suffix[BASE];

  sol_count = 0;
  for (i = 0; i < BASE; i = i + 1) {
    used[i] = 0;
    assigned[i] = 0;
  }

  unique_len = get_unique_chars(words, num_words, unique);
  if (unique_len > BASE || unique_len == 0) {
    *out_unique_len = 0;
    return 0;
  }
  *out_unique_len = unique_len;

  compute_weights(words, num_words, unique, unique_len, weights);
  sort_chars(unique, weights, unique_len);

  last_nz = 0;
  for (i = 0; i < (int)unique_len; i = i + 1) {
    if (weights[i] != 0) {
      last_nz = i;
    }
  }

  presolve(words, num_words, unique, unique_len, domains);
  precomp_branches(unique_len, weights, max_suffix, min_suffix);

  index = 0;
  sum_stack[0] = 0;
  next_digit[0] = 0;

  while (index >= 0) {
    if (index == (int)unique_len) {
      if (sum_stack[index] == 0) {
        save_solution(solutions, max_solutions, sol_count, assigned,
                      unique_len);
        sol_count = sol_count + 1;
      }
      index = index - 1;
      if (index >= 0) {
        used[assigned[index]] = 0;
      }
    } else {
      d = find_next_digit(index, sum_stack[index], next_digit[index],
                          weights[index], max_suffix[index], min_suffix[index],
                          last_nz, used, domains);
      if (d != -1) {
        assigned[index] = (unsigned char)d;
        used[d] = 1;
        sum_stack[index + 1] = sum_stack[index] + (weights[index] * (long)d);
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
  return sol_count;
}

void print_solutions(char words[][MAX_WORD_LEN], unsigned char num_words,
                     char unique[], unsigned char unique_len,
                     unsigned char solutions[], unsigned long count,
                     unsigned long max_to_print) {
  int w_i, k;
  unsigned long s, offset, limit;
  long num;
  unsigned char sol[BASE];

  limit = count;
  if (limit > max_to_print) {
    limit = max_to_print;
  }
  for (s = 0; s < limit; s = s + 1) {
    offset = s * BASE;
    for (k = 0; k < (int)unique_len; k = k + 1) {
      sol[k] = solutions[offset + k];
    }
    printf("  Sol #%lu: ", s + 1);
    for (w_i = 0; w_i < (int)num_words; w_i = w_i + 1) {
      num = word_to_number(words[w_i], unique, unique_len, sol);
      if (w_i == (int)num_words - 1) {
        printf("= %ld  [", num);
      } else if (w_i == (int)num_words - 2) {
        printf("%ld ", num);
      } else {
        printf("%ld + ", num);
      }
    }
    for (k = 0; k < (int)unique_len; k = k + 1) {
      if (k == (int)unique_len - 1) {
        printf("%c=%d]\n", unique[k], (int)sol[k]);
      } else {
        printf("%c=%d, ", unique[k], (int)sol[k]);
      }
    }
  }
  if (count > max_to_print) {
    printf("  ... and %lu more solutions\n", count - max_to_print);
  }
}
