#pragma once

#define BASE 10
#define MAX_WORD_LEN 16
#define SOLUTIONS_SIZE 4096

int find_index(char c, char unique[], unsigned char len);

unsigned char get_unique_chars(char words[][MAX_WORD_LEN],
                               unsigned char num_words, char unique[]);

void compute_weights(char words[][MAX_WORD_LEN], unsigned char num_words,
                     char unique[], unsigned char unique_len, long weights[]);

void sort_chars(char unique[], long weights[], unsigned char unique_len);

long word_to_number(char word[], char unique[], unsigned char unique_len,
                    unsigned char assigned_digits[]);

void presolve(char words[][MAX_WORD_LEN], unsigned char num_words,
              char unique[], unsigned char unique_len,
              unsigned char domains[BASE][BASE]);

void precomp_branches(unsigned char unique_len, long weights[],
                      long max_suffix[], long min_suffix[]);

int find_next_digit(int index, long sum, int start_digit, long w,
                    long max_suffix, long min_suffix, int last_nz,
                    unsigned char used[], unsigned char domains[BASE][BASE]);

void save_solution(unsigned char solutions[], unsigned long max_solutions,
                   unsigned long sol_count, unsigned char assigned[],
                   unsigned char unique_len);

unsigned long solve(char words[][MAX_WORD_LEN], unsigned char num_words,
                    char unique[], unsigned char *out_unique_len,
                    unsigned char solutions[], unsigned long max_solutions);

void print_solutions(char words[][MAX_WORD_LEN], unsigned char num_words,
                     char unique[], unsigned char unique_len,
                     unsigned char solutions[], unsigned long count,
                     unsigned long max_to_print);
