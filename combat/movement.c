#include "movement.h"
#include "utils.h"
#include "queries.h"
#include "battlefield.h"
#include <math.h>
#include <stdlib.h>

void execute_move(Unit* unit, Battlefield* field, int target_x, int target_y) {
    int dx = (target_x > unit->x) ? 1 : (target_x < unit->x) ? -1 : 0;
    int dy = (target_y > unit->y) ? 1 : (target_y < unit->y) ? -1 : 0;
    int steps = unit->movement;
    int cur_x = unit->x, cur_y = unit->y;

    // Move in X direction first
    while (steps > 0 && dx != 0) {
        int next_x = cur_x + dx;
        if (is_tile_walkable(field, next_x, cur_y, unit)) {
            cur_x = next_x;
            steps--;
        } else {
            break; // blocked by unit
        }
    }
    // Then Y direction
    while (steps > 0 && dy != 0) {
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
}

void execute_retreat(Unit* unit, Unit* target, Battlefield* field) {
    if (!target || unit->movement == 0 || unit->is_immobilized) return;
    int dx = unit->x - target->x;
    int dy = unit->y - target->y;
    // Normalize direction
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

    // Move in X direction first
    while (steps > 0 && step_x != 0) {
        int next_x = cur_x + step_x;
        if (is_tile_walkable(field, next_x, cur_y, unit)) {
            cur_x = next_x;
            steps--;
        } else {
            break; // blocked by obstacle or unit
        }
    }
    // Then Y direction
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
    if (!target || move_amount <= 0) return;

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

void execute_squad_move(Unit* unit, Battlefield* field, int target_x, int target_y, int move_amount) {
    if (!unit || !field || move_amount <= 0) return;
    int squad_id = unit->squad_id;
    if (squad_id == -1) return;

    // Gather squad members
    Unit* squad_units[MAX_UNITS];
    int squad_count = 0;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (other == NULL || other->hp <= 0) continue;
        if (other->squad_id == squad_id) {
            squad_units[squad_count++] = other;
        }
    }
    if (squad_count < 2) {
        execute_move(unit, field, target_x, target_y);
        return;
    }

    // Compute old centre
    int old_centre_x = 0, old_centre_y = 0;
    for (int i = 0; i < squad_count; i++) {
        old_centre_x += squad_units[i]->x;
        old_centre_y += squad_units[i]->y;
    }
    old_centre_x /= squad_count;
    old_centre_y /= squad_count;

    // Compute new centre (move toward target)
    int dx = target_x - old_centre_x;
    int dy = target_y - old_centre_y;
    int dist = abs(dx) + abs(dy);
    int new_centre_x = old_centre_x;
    int new_centre_y = old_centre_y;

    if (dist <= move_amount) {
        new_centre_x = target_x;
        new_centre_y = target_y;
    } else {
        int remaining = move_amount;
        if (dx != 0) {
            int step_x = (dx > 0) ? remaining : -remaining;
            if (abs(step_x) > abs(dx)) step_x = dx;
            new_centre_x = old_centre_x + step_x;
            remaining -= abs(step_x);
        }
        if (dy != 0 && remaining > 0) {
            int step_y = (dy > 0) ? remaining : -remaining;
            if (abs(step_y) > abs(dy)) step_y = dy;
            new_centre_y = old_centre_y + step_y;
            remaining -= abs(step_y);
        }
    }

    if (old_centre_x == new_centre_x && old_centre_y == new_centre_y) {
        // No change in centre, skip movement
        return;
    }

    // For each unit, compute offset from old centre and place at new centre + offset
    int temp_x[MAX_UNITS], temp_y[MAX_UNITS];
    for (int i = 0; i < squad_count; i++) {
        Unit* u = squad_units[i];
        int offset_x = u->x - old_centre_x;
        int offset_y = u->y - old_centre_y;
        int proposed_x = new_centre_x + offset_x;
        int proposed_y = new_centre_y + offset_y;
        // Clamp to battlefield
        if (proposed_x < 0) proposed_x = 0;
        if (proposed_x >= field->width) proposed_x = field->width - 1;
        if (proposed_y < 0) proposed_y = 0;
        if (proposed_y >= field->height) proposed_y = field->height - 1;
        temp_x[i] = proposed_x;
        temp_y[i] = proposed_y;
    }

    // Apply positions
    for (int i = 0; i < squad_count; i++) {
        squad_units[i]->x = temp_x[i];
        squad_units[i]->y = temp_y[i];
    }
    // Resolve overlaps for the squad
    resolve_overlaps(field);
}