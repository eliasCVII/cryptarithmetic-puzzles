#pragma once

#define BASE 10
#define PUZ_SIZE 16 // cambiar a 16?

int parse(char *p, char *buff[]);

int find_index(char c, char unique[], unsigned char len);

unsigned char get_unique_chars(char *words[PUZ_SIZE], unsigned char num_words,
                               char unique[]);

void compute_weights(char *words[PUZ_SIZE], unsigned char words_len,
                     char unique[], unsigned char unique_len, long weights[]);

void sort_chars(char unique[], long weights[], unsigned char unique_len);

void presolve(char *words[PUZ_SIZE], unsigned char words_len, char unique[],
              unsigned char unique_len, unsigned char domains[BASE][BASE]);

void compute_rem_bounds(unsigned char unique_len, long weights[],
                        long max_rem[], long min_rem[]);

int find_next_digit(int index, long sum, int start_digit, long w, long max_rem,
                    long min_rem, int last_nz, unsigned char used[],
                    unsigned char domains[BASE][BASE]);

unsigned long solve(char *words[PUZ_SIZE], unsigned char len, int print,
                    int first);
