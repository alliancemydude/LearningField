#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

// Weapon Types
typedef enum { 
    WEAPON_AXE, 
    WEAPON_SPEAR, 
    WEAPON_JAVELIN, 
    WEAPON_BOW,
    WEAPON_COUNT } WeaponType;

// Unit Types
typedef enum { 
    UNIT_BARBARIAN, 
    UNIT_ELITE_BARBARIAN, 
    UNIT_JAVELIN_THROWER,
    UNIT_GERMAN_CAVALRY, 
    UNIT_LEGIONARE,
    UNIT_CENTURION,
    UNIT_ARCHER, 
    UNIT_ROMAN_CAVALRY,
    UNIT_TYPE_COUNT } UnitType;

// Factions
typedef enum { 
    FACTION_GERMAN, 
    FACTION_ROMAN,
    FACTION_UNKNOWN
} Faction;

typedef enum {
    FACE_NORTH = 0,
    FACE_EAST  = 1,
    FACE_SOUTH = 2,
    FACE_WEST  = 3
} Facing;

typedef struct {
    UnitType type;
    int count;
} ForceComposition;

// Constants
#define MAX_UNITS 200
#define MAX_ROWS 60
#define MAX_COLS 48
#define MAX_TURNS 35
#define RANDOM_SEED true

extern bool track_actions;
extern bool track_average_actions;
extern bool turn_based;
extern bool print_battle_summary_flag;
extern bool print_stats_periodically;
extern int stats_print_interval;
extern int unit_point_values[UNIT_TYPE_COUNT];

#endif