#include "queries.h"
#include "utils.h"
#include "battlefield.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Handle finding overall parameters
Faction get_faction_of_unit(UnitType type) {
    switch (type) {
        case UNIT_SWORDSMAN:
        case UNIT_ELITE_SWORDSMAN:
        case UNIT_LONGBOWMAN:
        case UNIT_HORSEMAN:
            return FACTION_REPUBLIC;

        case UNIT_SPEARMAN:
        case UNIT_ELITE_SPEARMAN:
        case UNIT_SHORTBOWMAN:
        case UNIT_CAMELMAN:
            return FACTION_SEPARATIST;

        default:
            return FACTION_UNKNOWN;
    }
}

Faction enemy_faction(Faction f) {
    return (f == FACTION_REPUBLIC) ? FACTION_SEPARATIST : FACTION_REPUBLIC;
}

int get_turn_count() {
    return turn_count;
}

int get_weapon_range(Unit* unit) {
    int max_range = 0;
    for (int i = 0; i < unit->weapon_count; i++) {
        if (unit->weapons[i].range > max_range) {
            max_range = unit->weapons[i].range;
        }
    }
    return max_range;
}

// Handle finding enemies by type
Unit* find_closest_enemy(Unit* unit, Battlefield* field) {
    Unit* closest = NULL;
    int closest_distance_sq = 9999 * 9999;

    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (other == NULL) {
            continue;
        }

        // Skip self or dead units
        if (other == unit || other->hp <= 0) {
            continue;
        }

        // Skip allies
        if (other->faction == unit->faction) {
            continue;
        }

        // Choose closest enemy
        int distance_sq = get_distance_squared(unit, other);
        if (distance_sq < closest_distance_sq) {
            closest_distance_sq = distance_sq;
            closest = other;
        }
    }
    return closest;
}

int get_enemies_in_range(Unit* unit, Battlefield* field, int range, Unit* enemies[]) {
    int count = 0;
    int range_sq = range * range;

    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (other == NULL) continue;

        if (other == unit || other->hp <= 0) {
            continue;
        }
        if (other->faction == unit->faction) {
            continue;
        }

        if (get_distance_squared(unit, other) <= range_sq) {
            enemies[count++] = other;
            if (count >= MAX_UNITS) {
                break;
            }
        }
    }
    return count;
}

Unit* find_lowest_hp_enemy(Unit* unit, Battlefield* field, int weapon_range) {
    Unit* lowest_hp = NULL;

    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];

        // If targeting null, ourself, or dead units, skip
        if (!other || other == unit || other->hp <= 0) {
            continue;
        }

        // Skip over allies
        if (other->faction == unit->faction) {
            continue;
        }

        // Skip enemies out of range
        if (get_distance(unit, other) > weapon_range) {
            continue;
        }

        // Choose the lowest hp unit
        if (!lowest_hp || other->hp < lowest_hp->hp) {
            lowest_hp = other;
        }
    }

    return lowest_hp;
}

Unit* find_highest_pin_enemy(Unit* unit, Battlefield* field, int weapon_range) {
    Unit* highest_pins = NULL;

    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];

        // If targeting null, ourself, or dead units, skip
        if (!other || other == unit || other->hp <= 0) {
            continue;
        }

        // Skip over allies
        if (other->faction == unit->faction) {
            continue;
        }

        // Skip enemies out of range
        if (get_distance(unit, other) > weapon_range) {
            continue;
        }

        // Choose the highest pinned unit
        if (!highest_pins || other->pin_markers > highest_pins->pin_markers) {
            highest_pins = other;
        }
    }

    return highest_pins;
}

Unit* find_highest_value_enemy(Unit* unit, Battlefield* field, int weapon_range) {
    Unit* highest_value = NULL;

    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (!other || other == unit || other->hp <= 0) {
            continue;
        }

        // Skip allies
        if (other->faction == unit->faction) {
            continue;
        }

        // Skip enemies out of range
        if (get_distance(unit, other) > weapon_range) {
            continue;
        }

        // Choose highest value
        if (!highest_value || other->point_value > highest_value->point_value) {
            highest_value = other;
        }
    }

    return highest_value;
}

Unit* select_closest_unit(Battlefield* field, Faction faction) {
    Unit* best = NULL;
    int best_dist_sq = 9999 * 9999;

    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (!u || u->hp <= 0) continue;
        if (u->faction != faction) continue;
        if (u->has_acted) continue;

        // Inline scan for closest enemy
        int min_dist_sq = 9999 * 9999;
        for (int j = 0; j < field->unit_count; j++) {
            Unit* e = field->units[j];
            if (!e || e == u || e->hp <= 0) continue;
            if (e->faction == u->faction) continue;
            int d = get_distance_squared(u, e);
            if (d < min_dist_sq) min_dist_sq = d;
        }

        if (min_dist_sq < best_dist_sq) {
            best_dist_sq = min_dist_sq;
            best = u;
        }
    }
    return best;
}

void get_faction_center(Battlefield* field, Faction faction, int* out_x, int* out_y) {
    if (!field || !out_x || !out_y) return;

    int sum_x = 0, sum_y = 0, count = 0;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (!u || u->hp <= 0 || u->faction != faction) continue;
        sum_x += u->x;
        sum_y += u->y;
        count++;
    }

    if (count == 0) {
        // Fallback: map centre. Victory checks in run_battle should prevent this.
        *out_x = field->width / 2;
        *out_y = field->height / 2;
        return;
    }

    *out_x = sum_x / count;
    *out_y = sum_y / count;
}

// Handle finding allies by type
Unit* find_closest_ally(Unit* unit, Battlefield* field) {
    Unit* closest = NULL;
    int closest_distance_sq = 9999 * 9999;

    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (other == NULL) {
            continue;
        }

        // Skip self or dead units
        if (other == unit || other->hp <= 0) {
            continue;
        }

        // Skip enemies
        if (other->faction != unit->faction) {
            continue;
        }

        // Choose closest ally
        int distance_sq = get_distance_squared(unit, other);
        if (distance_sq < closest_distance_sq) {
            closest_distance_sq = distance_sq;
            closest = other;
        }
    }
    return closest;
}

int count_faction_in_radius(Unit* unit, Battlefield* field, Faction faction, int radius) {
    int count = 0;
    int radius_sq = radius * radius;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (!other || other == unit || other->hp <= 0) continue;
        if (other->faction != faction) continue;
        if (get_distance_squared(unit, other) <= radius_sq) count++;
    }
    return count;
}