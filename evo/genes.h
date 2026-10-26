#ifndef GENES_H
#define GENES_H

#include <stddef.h>

#define STRATEGY_GENE_COUNT 10
#define UNIT_TYPE_GENE_COUNT 5

typedef struct {
    const char* name;
    float min;
    float max;
    float default_val;
} GeneInfo;

extern const GeneInfo GERMAN_GENES[];
extern const size_t GERMAN_GENE_COUNT;
extern const GeneInfo ROMAN_GENES[];
extern const size_t ROMAN_GENE_COUNT;

#endif