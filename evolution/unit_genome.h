#ifndef UNIT_GENOME_H
#define UNIT_GENOME_H

#include "config.h"

typedef struct {
    float unit_aggression;            // 0–1
    float preferred_range;            // 0-36
    float spacing_x;                  // 1-8: minimum horizontal gap to nearest ally
    float spacing_y;                  // 1-8: minimum vertical gap to nearest ally
} UnitGenome;

extern UnitGenome unit_genomes[UNIT_TYPE_COUNT];

#endif