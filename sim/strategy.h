#ifndef STRATEGY_H
#define STRATEGY_H

#include "config.h"
#include "strategic_genome.h"
#include "deployment_genome.h"
#include "unit_genome.h"
#include "evolution.h"
#include "composition.h"

struct Unit;
struct Battlefield;

typedef struct {
    StrategicGenome strategic;
    DeploymentGenome deployment;
    CompositionGenome composition;
} FactionStrategy;

void assign_unit_roles(struct Battlefield* field, Faction faction);

extern FactionStrategy german_strategy;
extern FactionStrategy roman_strategy;

void set_german_strategy(const float weights[10]);
void set_roman_strategy(const float weights[10]);
void load_strategy_from_file(const char* filename);

PairFitness evaluate_matchup(const float* german_weights, const float* roman_weights, int num_battles);

#endif