#ifndef EVOLUTION_H
#define EVOLUTION_H

#include <stddef.h>
#include "config.h"

typedef enum {
    EVOLVE_TACTICAL = 1,
    EVOLVE_STRATEGIC = 2,
    EVOLVE_DEPLOYMENT = 4,
    EVOLVE_ALL = 7,
    EVOLVE_UNIT = 8
} EvolutionMode;

typedef enum {
    CAT_TACTICAL   = 1 << 0,
    CAT_STRATEGIC  = 1 << 1,
    CAT_DEPLOYMENT = 1 << 2,
    CAT_ALL        = CAT_TACTICAL | CAT_STRATEGIC | CAT_DEPLOYMENT
} GeneCategory;

typedef struct {
    const char* name;
    float min;
    float max;
    float default_val;
    GeneCategory category;
} GeneInfo;

typedef struct {
    float rep_fitness;
    float sep_fitness;
} PairFitness;

void run_evolution(EvolutionMode mode);
void print_genome(const float* w, const GeneInfo* genes, size_t gene_count, const char* name);
void save_genomes(const float* rep_weights, const float* sep_weights, const GeneInfo* genes, size_t gene_count, const char* prefix);
void print_unit_genome(const float* w, Faction faction, const char* name);
void save_unit_genome(const float* weights, Faction faction, const char* prefix);

PairFitness evaluate_unit_pair(const float* rep_weights, const float* sep_weights, int num_battles);

// Gene arrays
extern const GeneInfo STRATEGY_GENES[];
extern const size_t STRATEGY_GENE_COUNT;
extern const GeneInfo UNIT_GENES[];
extern const size_t UNIT_GENE_COUNT;
extern const size_t UNIT_TYPE_GENE_COUNT;

#endif