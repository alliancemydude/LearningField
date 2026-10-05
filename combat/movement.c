#include "movement.h"
#include "utils.h"
#include "queries.h"
#include "battlefield.h"
#include <math.h>
#include <stdlib.h>

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

    int cur_x = unit->x, cur_y = unit->y;
    int steps = max_steps;

    // Alternate axis order by unit id so the army doesn't collapse
    // into a single column. Even ids go X-then-Y, odd ids Y-then-X.
    bool y_first = (unit->id % 2 != 0);

    if (!y_first) {
        steps -= step_axis(unit, field, &cur_x, &cur_y, target_x, true,  steps);
        steps -= step_axis(unit, field, &cur_x, &cur_y, target_y, false, steps);
    } else {
        steps -= step_axis(unit, field, &cur_x, &cur_y, target_y, false, steps);
        steps -= step_axis(unit, field, &cur_x, &cur_y, target_x, true,  steps);
    }

    unit->x = cur_x;
    unit->y = cur_y;
    clamp_unit_position(unit, field);
}

void move_toward_target(Unit* unit, Unit* target, int move_amount, Battlefield* field) {
    if (!unit || !field) return;
    if (!target || move_amount <= 0) return;
    if (unit->is_immobilized) return;

    int cur_x = unit->x, cur_y = unit->y;
    int steps = move_amount;

    // Same alternating axis order as execute_move
    bool y_first = (unit->id % 2 != 0);

    if (!y_first) {
        steps -= step_axis(unit, field, &cur_x, &cur_y, target->x, true,  steps);
        steps -= step_axis(unit, field, &cur_x, &cur_y, target->y, false, steps);
    } else {
        steps -= step_axis(unit, field, &cur_x, &cur_y, target->y, false, steps);
        steps -= step_axis(unit, field, &cur_x, &cur_y, target->x, true,  steps);
    }

    unit->x = cur_x;
    unit->y = cur_y;
    clamp_unit_position(unit, field);
}