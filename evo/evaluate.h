#ifndef EVALUATE_H
#define EVALUATE_H

#include "ga.h"

void apply_genes(const float* german_weights, const float* roman_weights);
PairFitness evaluate_all(const float* german_weights, const float* roman_weights, int num_battles);
void print_generation_diagnostics(const float* german_weights, const float* roman_weights,
                                  int num_battles, const char* label);

#endif