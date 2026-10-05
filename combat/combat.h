#ifndef COMBAT_H
#define COMBAT_H

#include "unit.h"
#include "battlefield.h"

// Combat mechanics
int morale_check(Unit* unit);
int take_damage(Unit* unit, int kills);
void add_pin_marker(Unit* unit);

// Actions
bool execute_fire(Unit* unit, Unit* target, bool advanced);
bool execute_attack(Unit* unit, Unit* target, bool advanced);
bool execute_rally(Unit* unit);

#endif