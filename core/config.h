#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

// Weapon Types
typedef enum { 
    // Small arms
    WEAPON_BLASTER, 
    WEAPON_CARBINE, 
    WEAPON_SNIPER, 

    // Explosives
    WEAPON_GRENADE, 
    WEAPON_ROCKET,

    // Machine Guns
    WEAPON_SUB_MG,
    WEAPON_LIGHT_MG, 
    WEAPON_MEDIUM_MG,
    WEAPON_HEAVY_MG, 

    // Anti-tank weapons
    WEAPON_LIGHT_AT,
    WEAPON_MEDIUM_AT,
    WEAPON_BLASTER_CANNON,
    WEAPON_DUAL_BLASTER_CANNON,
    WEAPON_MISSILE_POD,
    WEAPON_BEAM_CANNON,
    WEAPON_COUNT } WeaponType;

// Unit Types
typedef enum { 
    // Republic Infantry
    UNIT_CLONE_TROOPER, 
    UNIT_ELITE_CLONE, 
    UNIT_CLONE_SNIPER,
    UNIT_CLONE_OFFICER, 
    UNIT_CLONE_ROCKET,

    // Republic Gunners
    UNIT_LIGHT_CLONE_GUNNER,
    UNIT_MEDIUM_CLONE_GUNNER, 
    UNIT_HEAVY_CLONE_GUNNER,

    // Republic Armor
    UNIT_SWAMP_SPEEDER, 
    UNIT_BARC_SPEEDER,
    UNIT_SABER_TANK,

    // Separatist Infantry
    UNIT_BATTLE_DROID,
    UNIT_SUPER_BATTLE_DROID, 
    UNIT_DROID_SNIPER,
    UNIT_DROID_OFFICER, 
    UNIT_DROID_ROCKET,

    // Separatist Gunners
    UNIT_MEDIUM_DROID_GUNNER,
    UNIT_HEAVY_DROID_GUNNER, 
    
    // Separatist Armor
    UNIT_SPIDER_DROID, 
    UNIT_STAP,
    UNIT_AAT,

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