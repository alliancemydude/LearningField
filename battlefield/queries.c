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

// Handle finding environmental factors
int find_nearest_cover(Unit* unit, Battlefield* field, int* target_x, int* target_y) {
    int closest_distance = 9999;
    int best_x = -1;
    int best_y = -1;
    
    // Loop through the entire battlefield
    for (int y = 0; y < field->height; y++) {
        for (int x = 0; x < field->width; x++) {
            // Check if this tile has cover
            if (field->battlefield[y][x] == TERRAIN_HALF_COVER || 
                field->battlefield[y][x] == TERRAIN_FULL_COVER) {
                
                // Check if any enemy is standing on this cover
                bool enemy_on_cover = false;
                for (int i = 0; i < field->unit_count; i++) {
                    Unit* other = field->units[i];
                    if (other == NULL) continue;
                    if (other->hp <= 0) continue;
                    if (other->faction != unit->faction && 
                        other->x == x && other->y == y) {
                        enemy_on_cover = true;
                        break;
                    }
                }
                
                // Skip if enemy is on this cover
                if (enemy_on_cover) continue;
                
                // Calculate distance to this cover
                int dx = unit->x - x;
                int dy = unit->y - y;
                int distance = dx*dx + dy*dy;
                
                if (distance < closest_distance) {
                    closest_distance = distance;
                    best_x = x;
                    best_y = y;
                }
            }
        }
    }
    
    if (best_x != -1 && best_y != -1) {
        *target_x = best_x;
        *target_y = best_y;
        return closest_distance;
    }
    
    return 9999;  // No cover found
}

int get_cover_bonus_between(Unit* shooter, Unit* target, Battlefield* field) {
    if (shooter == NULL || target == NULL || field == NULL) return 0;
    if (shooter == target) return 0;

    int x0 = shooter->x, y0 = shooter->y;
    int x1 = target->x, y1 = target->y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : (x0 > x1) ? -1 : 0;
    int sy = (y0 < y1) ? 1 : (y0 > y1) ? -1 : 0;
    int err = dx - dy;
    int cur_x = x0, cur_y = y0;
    int steps = 0;
    const int MAX_STEPS = 200;

    int last_cover_x = -1, last_cover_y = -1;
    int max_cover_bonus = 0;

    while (steps < MAX_STEPS) {
        steps++;
        if (cur_x == x0 && cur_y == y0) {
            // skip start tile
        } else if (cur_x == x1 && cur_y == y1) {
            // reached target, stop
            break;
        } else {
            // check intermediate tile
            if (cur_x >= 0 && cur_x < field->width && cur_y >= 0 && cur_y < field->height) {
                TerrainType terrain = field->battlefield[cur_y][cur_x];
                if (terrain == TERRAIN_HALF_COVER || terrain == TERRAIN_FULL_COVER) {
                    last_cover_x = cur_x;
                    last_cover_y = cur_y;
                    if (terrain == TERRAIN_FULL_COVER && max_cover_bonus < 2) {
                        max_cover_bonus = 2;
                    } else if (terrain == TERRAIN_HALF_COVER && max_cover_bonus < 1) {
                        max_cover_bonus = 1;
                    }
                }
            }
        }

        // Bresenham step
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            cur_x += sx;
        }
        if (e2 < dx) {
            err += dx;
            cur_y += sy;
        }

        if (cur_x < 0 || cur_x >= field->width || cur_y < 0 || cur_y >= field->height) break;
        if ((sx > 0 && cur_x > x1) || (sx < 0 && cur_x < x1) ||
            (sy > 0 && cur_y > y1) || (sy < 0 && cur_y < y1)) break;
    }

    // No cover found
    if (last_cover_x == -1) return 0;

    // Check if the defender is adjacent to the last cover tile (Manhattan distance <= 1)
    int dist_to_cover = abs(x1 - last_cover_x) + abs(y1 - last_cover_y);
    if (dist_to_cover <= 1) {
        return max_cover_bonus;  // 1 or 2
    } else {
        return -1;  // line of sight blocked
    }
}