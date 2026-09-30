#ifndef UNIT_H
#define UNIT_H

#include "config.h"
#include "strategy.h"
#include <stdbool.h>

// Weapon struct, where stats are kept
typedef struct {
    WeaponType type;
    int penetration_bonus;
    int damage_bonus;
    int range;
    bool equipped;
    int explosion_radius;
} Weapon;

// Unit struct
typedef struct Unit {
    // Identity
    int id;
    UnitType type;
    Faction faction;

    // Stats
    int hp;
    int max_hp;
    int hp_per_soldier;
    int movement;
    int half_movement;
    int morale;
    int armor_class;
    int point_value;
    bool is_armor;
    int units_count;
    int attacks_per_unit;
    int max_range;
    int max_range_sq;

    // Preferences
    int attack_order;
    int preferred_range;
    int preferred_range_sq;
    int danger_range;
    int danger_range_sq;

    // Genomes
    float effective_aggression;
    float effective_explosive_threshold;
    float effective_ally_proximity;
    float effective_retreat_hp_ratio;
    struct Unit* focus_target;

    // Status
    int rally_bonus;
    int pin_markers;
    bool has_acted;
    bool is_stunned;
    bool is_immobilized;

    // Formations
    int formation_index; // unit's position in formation, 0 = leader, 1 = right, 2 = left

    // Position
    int x, y;
    bool squad_moved;

    // Weapons
    Weapon weapons[WEAPON_COUNT];
    int weapon_count;
    int squad_id;
    int actions_taken;

    // Strategy
    StrategicOrder strategic_order;
} Unit;

// Create units
Unit* create_unit(UnitType type, int id, int x, int y);
Unit* create_unit_squad(UnitType type, int id, int x, int y);

// Useful functions
const char* get_unit_type_name(UnitType type);
void add_weapon(Unit* unit, WeaponType type);
void clear_weapons(Unit* unit);
void destroy_unit(Unit* unit);

#endif