#ifndef COMBAT_H
#define COMBAT_H

#include "unit.h"
#include "battlefield.h"

// Combat mechanics
int morale_check(Unit* unit, Battlefield* field);
bool has_explosives(Unit* unit);
int take_damage(Unit* unit, int kills, bool critical);
void add_pin_marker(Unit* unit);

// Actions
bool execute_fire(Unit* unit, Unit* target, bool advanced, Battlefield* field);
bool execute_explosive(Unit* unit, Unit* target, bool advanced, Battlefield* field);
bool execute_rally(Unit* unit, Battlefield* field);

#endif