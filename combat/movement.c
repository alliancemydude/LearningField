#include "movement.h"
#include "utils.h"
#include "queries.h"
#include "battlefield.h"
#include <math.h>
#include <stdlib.h>

static int dir_from_delta(int dx, int dy) {
    if (dx > 0) return FACE_EAST;
    if (dx < 0) return FACE_WEST;
    if (dy > 0) return FACE_SOUTH;
    return FACE_NORTH;
}

static int step_axis(Unit* unit, Battlefield* field, int* cur_x, int* cur_y, int target, bool is_x_axis, int steps) {
    int taken = 0;
    while (steps > 0) {
        int cur = is_x_axis ? *cur_x : *cur_y;
        if (cur == target) break;

        int dir = (target > cur) ? 1 : -1;
        int next_x = is_x_axis ? cur + dir : *cur_x;
        int next_y = is_x_axis ? *cur_y : cur + dir;

        if (is_tile_walkable(field, next_x, next_y, unit)) {
            *cur_x = next_x;
            *cur_y = next_y;
            // Record this step
            if (unit->path_len < (int)(sizeof(unit->path_x) / sizeof(unit->path_x[0]))) {
                unit->path_x[unit->path_len] = next_x;
                unit->path_y[unit->path_len] = next_y;
                unit->path_len++;
            }
            steps--;
            taken++;
        } else {
            break;
        }
    }
    return taken;
}

void execute_move(Unit* unit, Battlefield* field, int target_x, int target_y, int max_steps) {
    if (!unit || !field) return;
    if (unit->movement == 0 || unit->is_immobilized) return;

    int orig_x = unit->x, orig_y = unit->y;
    int cur_x = orig_x, cur_y = orig_y;
    int steps = max_steps;

    bool y_first = (unit->id % 2 != 0);

    if (!y_first) {
        steps -= step_axis(unit, field, &cur_x, &cur_y, target_x, true,  steps);
        steps -= step_axis(unit, field, &cur_x, &cur_y, target_y, false, steps);
    } else {
        steps -= step_axis(unit, field, &cur_x, &cur_y, target_y, false, steps);
        steps -= step_axis(unit, field, &cur_x, &cur_y, target_x, true,  steps);
    }

    field->occupancy[orig_y][orig_x] = NULL;

    unit->x = cur_x;
    unit->y = cur_y;
    clamp_unit_position(unit, field);

    // Set facing from the actual displacement
    int dx = unit->x - orig_x;
    int dy = unit->y - orig_y;
    if (dx != 0 || dy != 0) {
        unit->facing = dir_from_delta(dx, dy);
    }

    field->occupancy[unit->y][unit->x] = unit;
}

void move_toward_target(Unit* unit, Unit* target, int move_amount, Battlefield* field) {
    if (!unit || !field) return;
    if (!target || move_amount <= 0) return;
    if (unit->is_immobilized) return;

    int orig_x = unit->x, orig_y = unit->y;
    int cur_x = orig_x, cur_y = orig_y;
    int steps = move_amount;

    bool y_first = (unit->id % 2 != 0);

    if (!y_first) {
        steps -= step_axis(unit, field, &cur_x, &cur_y, target->x, true,  steps);
        steps -= step_axis(unit, field, &cur_x, &cur_y, target->y, false, steps);
    } else {
        steps -= step_axis(unit, field, &cur_x, &cur_y, target->y, false, steps);
        steps -= step_axis(unit, field, &cur_x, &cur_y, target->x, true,  steps);
    }

    field->occupancy[orig_y][orig_x] = NULL;

    unit->x = cur_x;
    unit->y = cur_y;
    clamp_unit_position(unit, field);

    int dx = unit->x - orig_x;
    int dy = unit->y - orig_y;
    if (dx != 0 || dy != 0) {
        unit->facing = dir_from_delta(dx, dy);
    }

    field->occupancy[unit->y][unit->x] = unit;
}