#ifndef COMPOSITION_H
#define COMPOSITION_H

#include "config.h"

// Total points each faction can spend per battle.
#define COMPOSITION_BUDGET 400

// Sizing for the share array. Current factions have 4 unit types each.
#define COMPOSITION_MAX_TYPES 8

typedef struct {
    float share[COMPOSITION_MAX_TYPES];
    int   count;                          // number of active slots
} CompositionGenome;

int build_composition(const CompositionGenome* genes, Faction faction, ForceComposition* out);

#endif