#ifndef UNIT_REGISTRY_H
#define UNIT_REGISTRY_H

#include "config.h"
#include <stdbool.h>


typedef struct {
    const char* short_name;   // "barbarian"
    const char* long_name;    // "UNIT_BARBARIAN"
    Faction     faction;
    int         point_value;

    // Stat block (was UnitBlueprint)
    int         hp;
    int         hp_per_soldier;
    int         movement;
    int         morale;
    int         armor_class;
    bool        is_ranged;
    int         units_count;
    int         attacks_per_unit;
    int         preferred_range;
    WeaponType  weapons[WEAPON_COUNT];
    int         weapon_count;
} UnitDefinition;

extern const UnitDefinition unit_registry[UNIT_TYPE_COUNT];

UnitType parse_unit_type_from_name(const char* short_name);

#endif