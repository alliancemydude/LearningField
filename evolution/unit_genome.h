#ifndef UNIT_GENOME_H
#define UNIT_GENOME_H

#include "config.h"

typedef struct {
    float unit_aggression;            // 0–1
    float unit_explosive_threshold;   // 1–4
    float unit_ally_proximity;        // 2–8
    float unit_retreat_hp_ratio;      // 0.1–0.6
    float preferred_range;            // 0-36
    float danger_range;               // 0-20
} UnitGenome;

extern UnitGenome unit_genomes[UNIT_TYPE_COUNT];

#endif