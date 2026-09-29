#include "queries.h"
#include "utils.h"
#include "battlefield.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Handle finding overall parameters
Faction get_faction_of_unit(UnitType type) {
    // Republic units: from 0 to UNIT_SABER_TANK inclusive
    if (type <= UNIT_SABER_TANK) {
        return FACTION_REPUBLIC;
    }
    // Separatist units: from UNIT_BATTLE_DROID to UNIT_AAT inclusive
    if (type >= UNIT_BATTLE_DROID && type <= UNIT_AAT) {
        return FACTION_SEPARATIST;
    }
    return FACTION_UNKNOWN;
}

int get_turn_count() {
    return turn_count;
}

bool is_faction_defeated(Battlefield* field, Faction faction) {
    for (int i = 0; i < field->unit_count; i++) {
        if (field->units[i]->faction == faction && field->units[i]->hp > 0) {
            return false;
        }
    }
    return true;
}

int get_weapon_range(Unit* unit) {
    int max_range = 0;
    for (int i = 0; i < unit->weapon_count; i++) {
        Weapon* w = &unit->weapons[i];
        int range = 0;
        switch (w->type) {
            // Small arms
            case WEAPON_BLASTER: range = 24; break;
            case WEAPON_CARBINE: range = 18; break;
            case WEAPON_SNIPER: range = 36; break;
            // Explosives
            case WEAPON_GRENADE: range = 12; break;
            case WEAPON_ROCKET: range = 16; break;
            // Machine guns
            case WEAPON_SUB_MG: range = 18; break;
            case WEAPON_LIGHT_MG: range = 24; break;
            case WEAPON_MEDIUM_MG: range = 36; break;
            case WEAPON_HEAVY_MG: range = 36; break;
            // Anti-Tank
            case WEAPON_LIGHT_AT: range = 48; break;
            case WEAPON_MEDIUM_AT: range = 60; break;
            // Vehicle cannons
            case WEAPON_BLASTER_CANNON: range = 48; break;
            case WEAPON_DUAL_BLASTER_CANNON: range = 48; break;
            case WEAPON_MISSILE_POD: range = 24; break;
            case WEAPON_BEAM_CANNON: range = 72; break;
            default: range = 24; break;
        }
        if (range > max_range) max_range = range;
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

Unit* find_highest_hp_enemy(Unit* unit, Battlefield* field) {
    Unit* highest = NULL;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (!other || other == unit || other->hp <= 0) {
            continue;
        }
        if (other->faction == unit->faction) {
            continue;
        }
        if (get_distance(unit, other) > (unit->movement + 6)) {
            continue;
        }
        if (!highest || other->hp > highest->hp) {
            highest = other;
        }
    }
    return highest;
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
        if (u == NULL) continue;
        if (u->hp <= 0) continue;
        if (u->faction != faction) continue;
        if (u->has_acted) continue;

        // Find the closest enemy to this unit
        Unit* enemy = find_closest_enemy(u, field);
        if (enemy == NULL) continue; // no enemies left? should not happen

        int dist_sq = get_distance_squared(u, enemy);
        if (dist_sq < best_dist_sq) {
            best_dist_sq = dist_sq;
            best = u;
        }
    }
    return best;
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

int get_distance_to_closest_ally(Unit* unit, Battlefield* field) {
    int closest_distance = 9999;
    
    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (other == NULL) continue;

        if (other == unit || other->hp <= 0) continue;
        if (other->faction != unit->faction) continue;
        
        int distance = get_distance(unit, other);
        if (distance < closest_distance) {
            closest_distance = distance;
        }
    }
    
    return closest_distance;
}

int get_distance_to_ally_type(Unit* unit, Battlefield* field, UnitType ally_type) {
    int closest_distance = 9999;
    
    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (other == NULL) continue;
        
        // Skip self
        if (other == unit) continue;
        
        // Skip dead units
        if (other->hp <= 0) continue;
        
        // Skip enemies (different faction)
        if (other->faction != unit->faction) continue;
        
        // Skip if not the type we're looking for
        if (other->type != ally_type) continue;
        
        // Calculate distance
        int distance = get_distance(unit, other);
        if (distance < closest_distance) {
            closest_distance = distance;
        }
    }
    
    // Return 9999 if no ally of that type found
    return closest_distance;
}

Unit* find_closest_pinned_ally(Unit* unit, Battlefield* field) {
    Unit* closest = NULL;
    int closest_distance = 9999;
    
    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (other == NULL) continue;

        if (other == unit || other->hp <= 0) continue;
        if (other->faction != unit->faction) continue;
        
        int distance = get_distance(unit, other);
        if (distance < closest_distance && other->pin_markers > 0) {
            closest_distance = distance;
            closest = other;
        }
    }
    
    return closest;  // Returns NULL if no ally of that type found
}

int get_distance_to_specific_ally(Unit* unit, Unit* target) {
    int distance = get_distance(unit, target);
    return distance;
}