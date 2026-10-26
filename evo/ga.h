#ifndef GA_H
#define GA_H

#include <stddef.h>
#include "genes.h"

extern int battles_min;
extern int battles_max;
extern int pop_size;
extern int num_generations;

typedef struct {
    float* weights;
    float fitness;
    float selection_score;
} Individual;

typedef struct {
    float german_fitness;
    float roman_fitness;
} PairFitness;

void run_coevolution(const GeneInfo* german_genes, size_t german_gene_count,
                     const GeneInfo* roman_genes, size_t roman_gene_count,
                     int pop_size, int num_generations,
                     PairFitness (*evaluate_pair)(const float*, const float*, int),
                     const char* name_prefix);

#endif