#ifndef UNIT_GENOME_H
#define UNIT_GENOME_H

#include "config.h"

typedef struct {
    float unit_aggression;            // 0–1 scaling of movement speed. How fast a unit closes the distance
    float preferred_range;            // 0-36 when a unit will stop closing distance. 
    float spacing_x;                  // 1-8 minimum horizontal distance to its nearest ally
    float spacing_y;                  // 1-8 minimum vertical distance to its nearest ally
} UnitGenome;

extern UnitGenome unit_genomes[UNIT_TYPE_COUNT];

#endif