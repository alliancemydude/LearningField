#ifndef BATTLEFIELD_H
#define BATTLEFIELD_H

#include "config.h"
#include "unit.h"
#include "strategy.h"

extern ForceComposition default_german_composition[];
extern int default_german_composition_count;
extern ForceComposition default_roman_composition[];
extern int default_roman_composition_count;

// Action tracking struct
typedef struct {
    int fire;
    int attack;
    int rally;
    int retreat;
    int dash;
    int cover;
    int advance_cover;
    int total;
} ActionCounts;

typedef struct Battlefield {
    int width, height;
    Unit* occupancy[MAX_ROWS][MAX_COLS];
    Unit* units[MAX_UNITS];
    int unit_count;
} Battlefield;

typedef struct {
    int winner;
    int shots_fired;
    int shots_hit;
    int kills;
    int flank_kills;
    int rear_kills;
    int damage_taken_german;
    int damage_taken_roman;
    int casualties_german;
    int casualties_roman;
    int kills_by_type[UNIT_TYPE_COUNT];
    int casualties_by_type[UNIT_TYPE_COUNT];
    int event_count;
} BattleStats;

extern BattleStats battle_stats;

// Declare the global variable
extern ActionCounts action_counts;
extern int total_actions_across_units;
extern int units_that_acted;
extern char battle_action_log[4096];
extern bool turn_based;
extern bool print_battle_summary_flag;

// Initialization:
void initialize_battlefield(Battlefield* field);
bool is_unit_used_in_composition(UnitType type);

// Start of battle:
int run_battle(Battlefield* field, int max_turns);

// Cleanup and stats:
void compact_dead_units(Battlefield* field);
void cleanup_battlefield(Battlefield* field);

#endif