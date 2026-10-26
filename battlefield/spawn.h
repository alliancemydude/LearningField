#ifndef SPAWN_H
#define SPAWN_H

#include "config.h"
#include "battlefield.h"

void resolve_overlaps(Battlefield* field);
void place_unit(Battlefield* field, Unit* u, int new_x, int new_y);
void spawn_german_forces(Battlefield* field, ForceComposition forces[], int force_count);
void spawn_roman_forces(Battlefield* field, ForceComposition forces[], int force_count);

#endif