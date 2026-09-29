#ifndef BATTLEFIELD_H
#define BATTLEFIELD_H

#include "config.h"
#include "unit.h"
#include "strategy.h"

typedef enum {
    MAP_OPEN,
    MAP_DEFENSIVE_REPUBLIC,
    MAP_DEFENSIVE_SEPARATIST,
    MAP_FLANKING,
    MAP_RIDGE,
    MAP_SCATTERED,
    MAP_COUNT
} MapType;

typedef struct {
    float aggr_mult;
    float advance_mult;   // if >1, units become more cautious (larger advance distance)
    float defend_mult;    // if >1, units defend more (larger defend distance)
} MapModifiers;

const MapModifiers* get_map_modifiers(MapType map, Faction faction);

typedef struct {
    UnitType type;
    int count;
} ForceComposition;

extern ForceComposition default_republic_composition[];
extern int default_republic_composition_count;
extern ForceComposition default_separatist_composition[];
extern int default_separatist_composition_count;

// Action tracking struct
typedef struct {
    int fire;
    int advance_fire;
    int explosive;
    int rally;
    int retreat;
    int dash;
    int cover;
    int advance_cover;
    int total;
} ActionCounts;

typedef struct Battlefield {
    int width, height;
    TerrainType battlefield[MAX_ROWS][MAX_COLS];
    Unit* units[MAX_UNITS];
    int unit_count;
    MapType map_type;
} Battlefield;

typedef struct {
    int winner;
    int shots_fired;
    int shots_hit;
    int kills;
    int explosives_used;
    int damage_dealt_rep;
    int damage_dealt_sep;
    int casualties_rep;
    int casualties_sep;
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
extern bool print_battle_summery_flag;

// Initialization:
void resolve_overlaps(Battlefield* field);
void initialize_battlefield(Battlefield* field);
void spawn_republic_forces(Battlefield* field, ForceComposition forces[], int force_count);
void spawn_separatist_forces(Battlefield* field, ForceComposition forces[], int force_count);
void spawn_cover_relative(Battlefield* field, float x_start, float x_end, float y_start, float y_end);
void spawn_cover(Battlefield* field, int x_min, int x_max, int y_min, int y_max);
void generate_terrain(Battlefield* field);
bool is_unit_used_in_composition(UnitType type);

// Start of battle:
int run_battle(Battlefield* field, int max_turns);

// Cleanup and stats:
void cleanup_battlefield(Battlefield* field);
void reset_stats(void);
void print_action_stats(void);
void print_battle_summary();
void print_battle_summary_and_wait(void);

#endif