#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

// Weapon Types
typedef enum { 
    WEAPON_SWORD, 
    WEAPON_SPEAR, 
    WEAPON_LONGBOW, 
    WEAPON_SHORTBOW,
    WEAPON_COUNT } WeaponType;

// Unit Types
typedef enum { 
    UNIT_SWORDSMAN, 
    UNIT_ELITE_SWORDSMAN, 
    UNIT_LONGBOWMAN,
    UNIT_HORSEMAN, 
    UNIT_SPEARMAN,
    UNIT_ELITE_SPEARMAN,
    UNIT_SHORTBOWMAN, 
    UNIT_CAMELMAN,
    UNIT_TYPE_COUNT } UnitType;

// Factions
typedef enum { 
    FACTION_REPUBLIC, 
    FACTION_SEPARATIST,
    FACTION_UNKNOWN
} Faction;

// Constants
#define MAX_UNITS 100
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