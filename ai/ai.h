#ifndef AI_H
#define AI_H
#include "unit.h"
#include "battlefield.h"
#include "combat.h"
#include "strategy.h"

// AI action types
typedef enum {
    ACTION_FIRE,
    ACTION_ADVANCE_FIRE,
    ACTION_ATTACK,
    ACTION_ADVANCE_ATTACK,
    ACTION_RALLY,
    ACTION_DASH,
    ACTION_RETREAT,
    ACTION_COVER,
    ACTION_ADVANCE_COVER
} ActionType;

typedef struct {
    ActionType type;
    Unit* target;      // For targeting enemies
    int target_x;      // For movement
    int target_y;      // For movement
} Action;

// Individual logic functions
int compare_enemies_by_preference(const Unit* a, const Unit* b, int preference_digit);
Unit* select_enemy_by_preference(Unit* unit, Battlefield* field, int max_range, StrategicGenome* strat);

bool evaluate_enemies(Unit* unit, Battlefield* field, char type, FactionStrategy* strat);

// Strategy
void unit_turn(Unit* unit, Battlefield* field, StrategicOrder* order);

#endif