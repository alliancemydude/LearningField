#include "movement.h"
#include "utils.h"
#include "queries.h"
#include "battlefield.h"
#include <math.h>
#include <stdlib.h>

void execute_move(Unit* unit, Battlefield* field, int target_x, int target_y, int max_steps) {
    if (!unit || !field) return;
    if (unit->movement == 0 || unit->is_immobilized) return;

    int cur_x = unit->x, cur_y = unit->y;
    int steps = max_steps;

    // Move in X direction first, but stop at target_x
    while (steps > 0 && cur_x != target_x) {
        int dx = (target_x > cur_x) ? 1 : -1;
        int next_x = cur_x + dx;
        if (is_tile_walkable(field, next_x, cur_y, unit)) {
            cur_x = next_x;
            steps--;
        } else {
            break;
        }
    }

    // Then Y direction, stop at target_y
    while (steps > 0 && cur_y != target_y) {
        int dy = (target_y > cur_y) ? 1 : -1;
        int next_y = cur_y + dy;
        if (is_tile_walkable(field, cur_x, next_y, unit)) {
            cur_y = next_y;
            steps--;
        } else {
            break;
        }
    }

    unit->x = cur_x;
    unit->y = cur_y;
    clamp_unit_position(unit, field);
}

void execute_retreat(Unit* unit, Unit* target, Battlefield* field) {
    if (!target || unit->movement == 0 || unit->is_immobilized) return;
    int dx = unit->x - target->x;
    int dy = unit->y - target->y;
    int step_x = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int step_y = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    int steps = unit->movement / 2;
    if (steps < 1) steps = 1;
    int cur_x = unit->x, cur_y = unit->y;

    while (steps > 0 && step_x != 0) {
        int next_x = cur_x + step_x;
        if (is_tile_walkable(field, next_x, cur_y, unit)) {
            cur_x = next_x;
            steps--;
        } else break;
    }
    while (steps > 0 && step_y != 0) {
        int next_y = cur_y + step_y;
        if (is_tile_walkable(field, cur_x, next_y, unit)) {
            cur_y = next_y;
            steps--;
        } else break;
    }
    unit->x = cur_x;
    unit->y = cur_y;
    clamp_unit_position(unit, field);
}

void execute_dash(Unit* unit, Unit* target, Battlefield* field) {
    if (!target || unit->movement == 0 || unit->is_immobilized) return;

    int dx = target->x - unit->x;
    int dy = target->y - unit->y;
    int step_x = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int step_y = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    int steps = unit->movement;
    int cur_x = unit->x, cur_y = unit->y;

    while (steps > 0 && step_x != 0) {
        int next_x = cur_x + step_x;
        if (is_tile_walkable(field, next_x, cur_y, unit)) {
            cur_x = next_x;
            steps--;
        } else {
            break;
        }
    }
    while (steps > 0 && step_y != 0) {
        int next_y = cur_y + step_y;
        if (is_tile_walkable(field, cur_x, next_y, unit)) {
            cur_y = next_y;
            steps--;
        } else {
            break;
        }
    }

    unit->x = cur_x;
    unit->y = cur_y;
    clamp_unit_position(unit, field);
}

void move_toward_target(Unit* unit, Unit* target, int move_amount, Battlefield* field) {
    if (!unit || !field) return;
    if (!target || move_amount <= 0) return;
    if (unit->is_immobilized) return;

    int dx = target->x - unit->x;
    int dy = target->y - unit->y;
    int step_x = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int step_y = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    int steps = move_amount;
    int cur_x = unit->x, cur_y = unit->y;

    while (steps > 0 && step_x != 0) {
        int next_x = cur_x + step_x;
        if (is_tile_walkable(field, next_x, cur_y, unit)) {
            cur_x = next_x;
            steps--;
        } else {
            break;
        }
    }
    while (steps > 0 && step_y != 0) {
        int next_y = cur_y + step_y;
        if (is_tile_walkable(field, cur_x, next_y, unit)) {
            cur_y = next_y;
            steps--;
        } else {
            break;
        }
    }

    unit->x = cur_x;
    unit->y = cur_y;
    clamp_unit_position(unit, field);
}