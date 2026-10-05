#ifndef QUERIES_H
#define QUERIES_H

#include "unit.h"
#include "battlefield.h"

// General queries:
int get_turn_count(void);
Faction get_faction_of_unit(UnitType type);
int get_weapon_range(Unit* unit);
Faction enemy_faction(Faction f);
void get_faction_center(Battlefield* field, Faction faction, int* out_x, int* out_y);

// Enemy queries:
Unit* find_closest_enemy(Unit* unit, Battlefield* field);
Unit* find_lowest_hp_enemy(Unit* unit, Battlefield* field, int weapon_range);
Unit* find_highest_pin_enemy(Unit* unit, Battlefield* field, int weapon_range);
Unit* find_highest_value_enemy(Unit* unit, Battlefield* field, int weapon_range);
int get_enemies_in_range(Unit* unit, Battlefield* field, int range, Unit* enemies[]);
Unit* select_closest_unit(Battlefield* field, Faction faction);

// Ally queries:
Unit* find_closest_ally(Unit* unit, Battlefield* field);
int count_faction_in_radius(Unit* unit, Battlefield* field, Faction faction, int radius);

#endif