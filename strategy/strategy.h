#ifndef STRATEGY_H
#define STRATEGY_H

#include "config.h"
#include "strategic_genome.h"
#include "deployment_genome.h"
#include "unit_genome.h"
#include "evolution.h"      // for PairFitness

struct Unit;
struct Battlefield;

typedef struct {
    StrategicGenome strategic;
    DeploymentGenome deployment;
} FactionStrategy;

void assign_unit_roles(struct Battlefield* field, Faction faction);

extern FactionStrategy republic_strategy;
extern FactionStrategy separatist_strategy;

void set_republic_strategy(const float weights[6]);
void set_separatist_strategy(const float weights[6]);
void load_strategy_from_file(const char* filename);

PairFitness evaluate_matchup(const float* rep_weights, const float* sep_weights, int num_battles);

#endif