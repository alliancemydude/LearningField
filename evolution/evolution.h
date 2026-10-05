#ifndef EVOLUTION_H
#define EVOLUTION_H

#include <stddef.h>
#include "config.h"

#define STRATEGY_GENE_COUNT 7
#define UNIT_TYPE_GENE_COUNT 5

typedef struct {
    const char* name;
    float min;
    float max;
    float default_val;
} GeneInfo;

typedef struct {
    float rep_fitness;
    float sep_fitness;
} PairFitness;

extern const GeneInfo REP_GENES[];
extern const size_t REP_GENE_COUNT;
extern const GeneInfo SEP_GENES[];
extern const size_t SEP_GENE_COUNT;

void run_evolution(void);
void print_genome(const float* w, const GeneInfo* genes, size_t gene_count, const char* name);
void save_genomes(const float* rep_weights, size_t rep_count,
                  const float* sep_weights, size_t sep_count,
                  const GeneInfo* rep_genes, const GeneInfo* sep_genes,
                  const char* prefix);

#endif