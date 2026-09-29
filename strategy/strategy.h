#ifndef STRATEGY_H
#define STRATEGY_H

#include "config.h"
#include "tactical_genome.h"
#include "strategic_genome.h"
#include "deployment_genome.h"
#include "unit_genome.h"

// Forward declarations
struct Unit;
struct Battlefield;

// Strategic Orders
typedef enum {
    ORDER_NONE,
    ORDER_ADVANCE,
    ORDER_DEFEND,
    ORDER_RETREAT,
    ORDER_MOVE_TO,
    ORDER_FOCUS_FIRE,
    ORDER_FLANK_LEFT,
    ORDER_FLANK_RIGHT,
    ORDER_BREAKTHROUGH,
    ORDER_FALLBACK,
    ORDER_SUPPRESS, // fire at enemies for pinning without advance
} StrategicOrderType;

typedef struct {
    StrategicOrderType type;
    int target_x;
    int target_y;
} StrategicOrder;

// Faction Strategy
typedef struct {
    TacticalGenome tactical;
    StrategicGenome strategic;
    DeploymentGenome deployment;
} FactionStrategy;

// Prototypes
bool should_use_formation(struct Unit* u);
void update_strategic_orders(struct Battlefield* field);
void assign_faction_orders(struct Battlefield* field, Faction faction);
void apply_squad_cohesion(struct Unit* acting_unit, struct Battlefield* field, FactionStrategy* strat);

// Global strategies
extern FactionStrategy republic_strategy;
extern FactionStrategy separatist_strategy;

// 
void set_republic_strategy(const float weights[22]);
void set_separatist_strategy(const float weights[22]);
void load_strategy_from_file(const char* filename);

float evaluate_matchup(const float* rep_weights, const float* sep_weights, int num_battles);

#endif