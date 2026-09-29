#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "unit.h"
#include "battlefield.h"

void execute_move(Unit* unit, Battlefield* field, int target_x, int target_y);
void execute_retreat(Unit* unit, Unit* target, Battlefield* field);
void execute_dash(Unit* unit, Unit* target, Battlefield* field);
void move_toward_target(Unit* unit, Unit* target, int move_amount, Battlefield* field);
void execute_squad_move(Unit* unit, Battlefield* field, int target_x, int target_y, int move_amount);

#endif