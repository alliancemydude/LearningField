#ifndef UNIT_BLUEPRINT_H
#define UNIT_BLUEPRINT_H

#include "unit.h"
#include "config.h"

// Weapon unit struct
typedef struct {
    WeaponType type;
} WeaponBlueprint;

// All unit stats struct. Does not interact with the battlefield,
// so things like position or cover are not here
typedef struct {
    UnitType type;
    Faction faction;
    int hp;
    int hp_per_soldier;
    int movement;
    int morale;
    int armor_class;
    int point_value;
    bool is_ranged;
    int units_count;
    int attacks_per_unit;
    int preferred_range;
    WeaponBlueprint weapons[WEAPON_COUNT];
    int weapon_count;
} UnitBlueprint;

extern const UnitBlueprint unit_blueprints[UNIT_TYPE_COUNT];

#endif