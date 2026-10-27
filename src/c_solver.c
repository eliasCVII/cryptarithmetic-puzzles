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

#define BASE 10
#define MAX_WORD_LEN 16
#define SOLUTIONS_SIZE 4096

struct String {
  char chars[MAX_WORD_LEN];
  unsigned char len;
};

struct Domain {
  unsigned char allowed[10];
  unsigned char count;
};

struct Stats {
  unsigned long nodes_visited;
  unsigned long conflict;
  unsigned long solution_count;
  char unique_chars[BASE];
  unsigned char unique_len;
};

struct Solver {
  long *weights;
  char *unique_chars;
  struct Domain *domains;
  long *max_suffix;
  long *min_suffix;
  unsigned char *assigned_digits;
  unsigned char *used_digits;
  unsigned char *solutions;
  unsigned long max_solutions;
  int last_nz;
  unsigned char unique_len;
  struct Stats *stats;
};

void domain_init(struct Domain *d) {
  int i;
  d->count = 10;
  for (i = 0; i < 10; i = i + 1) {
    d->allowed[i] = 1;
  }
}

void domain_remove(struct Domain *d, int digit) {
  if (d->allowed[digit] == 1) {
    d->allowed[digit] = 0;
    d->count = d->count - 1;
  }
}

void domain_fix(struct Domain *d, int digit) {
  int i;
  for (i = 0; i < 10; i = i + 1) {
    d->allowed[i] = 0;
  }
  d->allowed[digit] = 1;
  d->count = 1;
}

int domain_is_fixed(struct Domain *d) { return d->count == 1; }

int domain_get_fixed(struct Domain *d) {
  int i;
  for (i = 0; i < 10; i = i + 1) {
    if (d->allowed[i] == 1) {
      return i;
    }
  }
  return -1;
}

int domain_contains(struct Domain *d, int digit) {
  return d->allowed[digit] == 1;
}

int find_index(char c, char unique[], unsigned char len) {
  int i;

  for (i = 0; i < len; i = i + 1) {
    if (c == unique[i])
      return i;
  }
  return -1;
}

struct String get_unique_chars(struct String words[], unsigned char num_words) {
  struct String word, result = {0};
  int i, j;
  unsigned char ch;

  for (i = 0; i < num_words; i = i + 1) {
    word = words[i];
    for (j = 0; j < word.len; j = j + 1) {
      ch = word.chars[j];
      if (find_index(ch, result.chars, result.len) == -1) {
        if (result.len >= 10)
          break;
        result.chars[result.len] = ch;
        result.len = result.len + 1;
      }
    }
  }
  return result;
}

void compute_weights(struct String words[], unsigned char num_words,
                     char unique[], unsigned char unique_len, long weights[]) {
  int i, j, index;
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
    for (j = words[i].len - 1; j >= 0; j = j - 1) {
      ch = words[i].chars[j];
      index = find_index(ch, unique, unique_len);
      weights[index] = weights[index] + m;
      m = m * 10;
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

long word_to_number(struct String word, char unique[], unsigned char unique_len,
                    unsigned char assigned_digits[]) {
  long result;
  int i, index;

  result = 0;
  for (i = 0; i < word.len; i = i + 1) {
    index = find_index(word.chars[i], unique, unique_len);
    result = result * 10 + (long)assigned_digits[index];
  }
  return result;
}

void presolve(struct String words[], unsigned char num_words, char unique[],
              unsigned char unique_len, struct Domain domains[]) {
  int i, k, j, letter_pos, target_ch, fixed_digit;
  unsigned char max_len;

  for (i = 0; i < unique_len; i = i + 1) {
    domain_init(&domains[i]);
  }
  for (i = 0; i < num_words; i = i + 1) {
    if (words[i].len > 1) {
      letter_pos = find_index(words[i].chars[0], unique, unique_len);
      domain_remove(&domains[letter_pos], 0);
    }
  }
  max_len = 0;
  for (i = 0; i < num_words - 1; i = i + 1) {
    if (words[i].len > max_len) {
      max_len = words[i].len;
    }
  }
  if (num_words - 1 == 2 && words[num_words - 1].len > max_len) {
    target_ch = find_index(words[num_words - 1].chars[0], unique, unique_len);
    domain_fix(&domains[target_ch], 1);
  }
  for (k = 0; k < unique_len; k = k + 1) {
    if (domain_is_fixed(&domains[k])) {
      fixed_digit = domain_get_fixed(&domains[k]);
      for (j = 0; j < unique_len; j = j + 1) {
        if (j != k) {
          domain_remove(&domains[j], fixed_digit);
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

void search(struct Solver *s, unsigned char index, long current_sum) {
  int d, k;
  long w, d_long;
  unsigned long offset;

  s->stats->nodes_visited = s->stats->nodes_visited + 1;
  if (index == s->unique_len) {
    if (current_sum == 0) {
      if (s->stats->solution_count < s->max_solutions) {
        offset = s->stats->solution_count * 10;
        for (k = 0; k < s->unique_len; k = k + 1) {
          s->solutions[offset + k] = s->assigned_digits[k];
        }
      }
      s->stats->solution_count = s->stats->solution_count + 1;
    }
    return;
  }
  if (current_sum + s->max_suffix[index] < 0) {
    s->stats->conflict = s->stats->conflict + 1;
    return;
  }
  if (current_sum + s->min_suffix[index] > 0) {
    s->stats->conflict = s->stats->conflict + 1;
    return;
  }
  w = s->weights[index];
  if (index == s->last_nz && w != 0) {
    if ((-current_sum) % w == 0) {
      d_long = (-current_sum) / w;
      if (d_long >= 0 &&
          d_long <= 9 &&
          s->used_digits[d_long] == 0 &&
          domain_contains(&s->domains[index], (int)d_long) == 1)
      {
        d = (int)d_long;
        s->assigned_digits[index] = (unsigned char)d;
        s->used_digits[d] = 1;
        search(s, index + 1, current_sum + (w * d_long));
        s->used_digits[d] = 0;
      }
    }
    return;
  }
  for (d = 0; d <= 9; d = d + 1) {
    if (s->used_digits[d] == 0 && domain_contains(&s->domains[index], d) == 1) {
      s->assigned_digits[index] = (unsigned char)d;
      s->used_digits[d] = 1;
      search(s, index + 1, current_sum + (w * (long)d));
      s->used_digits[d] = 0;
    }
  }
}

struct Stats solve(
  struct String words[],
  unsigned char num_words,
  unsigned char solutions[],
  unsigned long max_solutions
) {
  struct String unique_word;
  struct Stats stats;
  struct Domain domains[BASE];
  struct Solver s;
  long weights[BASE], max_suffix[BASE], min_suffix[BASE];
  unsigned char assigned_digits[BASE], used_digits[BASE];
  int i, last_nz;

  stats.nodes_visited = 0;
  stats.conflict = 0;
  stats.solution_count = 0;
  unique_word = get_unique_chars(words, num_words);
  if (unique_word.len > 10) {
    stats.unique_len = 0;
    return stats;
  }
  stats.unique_len = unique_word.len;
  for (i = 0; i < unique_word.len; i = i + 1) {
    stats.unique_chars[i] = unique_word.chars[i];
  }
  compute_weights(words, num_words, stats.unique_chars, stats.unique_len,
                  weights);
  sort_chars(stats.unique_chars, weights, stats.unique_len);
  last_nz = 0;
  for (i = 0; i < stats.unique_len; i = i + 1) {
    if (weights[i] != 0) {
      last_nz = i;
    }
  }
  presolve(words, num_words, stats.unique_chars, stats.unique_len, domains);
  precomp_branches(stats.unique_len, weights, max_suffix, min_suffix);
  for (i = 0; i < 10; i = i + 1) {
    used_digits[i] = 0;
    assigned_digits[i] = 0;
  }
  s.weights = weights;
  s.unique_chars = stats.unique_chars;
  s.domains = domains;
  s.max_suffix = max_suffix;
  s.min_suffix = min_suffix;
  s.assigned_digits = assigned_digits;
  s.used_digits = used_digits;
  s.solutions = solutions;
  s.max_solutions = max_solutions;
  s.stats = &stats;
  s.last_nz = last_nz;
  s.unique_len = stats.unique_len;
  search(&s, 0, 0);
  return stats;
}

struct String make_word(char str[]) {
  struct String w;
  int i;

  w.len = 0;
  for (i = 0; str[i] != '\0'; i = i + 1) {
    w.chars[i] = str[i];
    w.len = w.len + 1;
  }
  w.chars[w.len] = '\0';
  return w;
}

void run_puzzle(char name[], struct String words[], unsigned char num_words) {
  struct timespec start_time, end_time;
  struct Stats stats;
  unsigned char solutions[SOLUTIONS_SIZE * 10];
  double elapsed_s, elapsed_us;
  unsigned long sol_idx;
  int w_i, k;
  long num;
  unsigned char sol[10];

  clock_gettime(CLOCK_MONOTONIC, &start_time);
  stats = solve(words, num_words, solutions, SOLUTIONS_SIZE);
  clock_gettime(CLOCK_MONOTONIC, &end_time);

  elapsed_s = (double)(end_time.tv_sec - start_time.tv_sec) +
              (double)(end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
  elapsed_us = elapsed_s * 1000000.0;

  printf("\n>> %s: %.6f s (%.2f us)\n", name, elapsed_s, elapsed_us);
  printf("   Solutions found : %lu\n", stats.solution_count);
  printf("   Nodes visited   : %lu\n", stats.nodes_visited);
  printf("   Conflicts       : %lu\n", stats.conflict);

  if (stats.solution_count > 0) {
    for (sol_idx = 0; sol_idx < stats.solution_count; sol_idx = sol_idx + 1) {
      if (sol_idx < 5) {
        for (k = 0; k < stats.unique_len; k = k + 1) {
          sol[k] = solutions[(sol_idx * 10) + k];
        }
        printf("   Sol #%lu: ", sol_idx + 1);
        for (w_i = 0; w_i < num_words; w_i = w_i + 1) {
          num = word_to_number(words[w_i], stats.unique_chars, stats.unique_len, sol);
          if (w_i == num_words - 1) {
            printf("= %ld\n", num);
          } else if (w_i == num_words - 2) {
            printf("%ld ", num);
          } else {
            printf("%ld + ", num);
          }
        }
      }
    }
    if (stats.solution_count > 5) {
      printf("   ... and %lu more solutions\n", stats.solution_count - 5);
    }
  }
}

int main(void) {
  struct String send_more_money[3];
  struct String cp_is_fun_true[4];
  struct String so_many_more[3];
  struct String ab_cd_ef_ghi[4];

  send_more_money[0] = make_word("SEND");
  send_more_money[1] = make_word("MORE");
  send_more_money[2] = make_word("MONEY");

  cp_is_fun_true[0] = make_word("CP");
  cp_is_fun_true[1] = make_word("IS");
  cp_is_fun_true[2] = make_word("FUN");
  cp_is_fun_true[3] = make_word("TRUE");

  so_many_more[0] = make_word("SO");
  so_many_more[1] = make_word("MANY");
  so_many_more[2] = make_word("MORE");

  ab_cd_ef_ghi[0] = make_word("AB");
  ab_cd_ef_ghi[1] = make_word("CD");
  ab_cd_ef_ghi[2] = make_word("EF");
  ab_cd_ef_ghi[3] = make_word("GHI");

  printf("=== CRYPTARITHMETIC PUZZLE SOLVER (C VERSION) ===\n");

  run_puzzle("SEND + MORE = MONEY", send_more_money, 3);
  run_puzzle("CP + IS + FUN = TRUE", cp_is_fun_true, 4);
  run_puzzle("SO + MANY = MORE", so_many_more, 3);
  run_puzzle("AB + CD + EF = GHI", ab_cd_ef_ghi, 4);

  return 0;
}
