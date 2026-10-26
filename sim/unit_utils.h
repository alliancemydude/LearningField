#ifndef UNIT_UTILS_H
#define UNIT_UTILS_H

#include "unit.h"

void clear_weapons(Unit* unit);
void add_weapon(Unit* unit, WeaponType type);
const char* short_unit_name(UnitType type);

#endif