#ifndef EVOLUTION_H
#define EVOLUTION_H

#include <stddef.h>
#include "config.h"

#define STRATEGY_GENE_COUNT 10
#define UNIT_TYPE_GENE_COUNT 5

typedef struct {
    const char* name;
    float min;
    float max;
    float default_val;
} GeneInfo;

typedef struct {
    float german_fitness;
    float roman_fitness;
} PairFitness;

extern const GeneInfo GERMAN_GENES[];
extern const size_t GERMAN_GENE_COUNT;
extern const GeneInfo ROMAN_GENES[];
extern const size_t ROMAN_GENE_COUNT;

void run_evolution(void);
void print_genome(const float* w, const GeneInfo* genes, size_t gene_count, const char* name);
void save_genomes(const float* german_weights, size_t german_count,
                  const float* roman_weights, size_t roman_count,
                  const GeneInfo* german_genes, const GeneInfo* roman_genes,
                  const char* prefix);

#endif