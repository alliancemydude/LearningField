#include "ai.h"
#include "battlefield.h"
#include "strategy.h"
#include "queries.h"
#include "utils.h"
#include "movement.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// Global variable for the target
static Unit* target = NULL;

// Evaluations
bool evaluate_enemies(Unit* unit, Battlefield* field, char type, FactionStrategy* strat, float effective_threshold) {
    switch(type) {
        case 'e': // Explosives
            // Check for explosives
            if (!has_explosives(unit)) {
                return false;
            }

            // Find the closest enemy, if they exist
            target = find_closest_enemy(unit, field);
            if (!target) {
                return false;
            }

            // Get all possible splash damage
            Unit* nearby[MAX_UNITS];
            int enemy_count = get_enemies_in_range(target, field, 2, nearby);
            int total_nearby = enemy_count + 1;
            if (total_nearby < (int)effective_threshold) {
                return false; // not enough enemies clustered
            }

            // Find the maximum range among all explosive weapons (grenade or rocket)
            int max_explosive_range = 0;
            for (int i = 0; i < unit->weapon_count; i++) {
                Weapon* w = &unit->weapons[i];
                if (w->ammo <= 0) {
                    continue;
                }
                if (w->type == WEAPON_GRENADE || w->type == WEAPON_ROCKET) {
                    if (w->range > max_explosive_range) {
                        max_explosive_range = w->range;
                    }
                }
            }
            if (max_explosive_range == 0) {
                return false;
            }

            // Select the highest-HP enemy within that range
            Unit* best = NULL;
            int best_hp = -1;
            for (int i = 0; i < field->unit_count; i++) {
                Unit* other = field->units[i];
                if (!other || other == unit || other->hp <= 0) {
                    continue;
                }
                if (other->faction == unit->faction) {
                    continue;
                }
                int d = get_distance_squared(unit, other);
                if (d > max_explosive_range * max_explosive_range || d <= 1) {
                    continue; // avoid suicide
                }
                if (other->hp > best_hp) {
                    best_hp = other->hp;
                    best = other;
                }
            }
            target = best;
            return (target != NULL);


        case 'f': // Fire
            int max_range = unit->max_range;
            if (max_range == 0) {
                return false;
            }
            target = select_enemy_by_preference(unit, field, max_range, &strat->strategic);
            return (target != NULL);


        case 'c': // Cover
        // Get the distance to the closest enemy
            Unit* closest_enemy = find_closest_enemy(unit, field);
            if (!closest_enemy) {
                return false;
            }
            int current_dist_to_enemy = get_distance(unit, closest_enemy);

            int best_cover_dist = 9999;
            int best_x = -1;
            for (int y = 0; y < field->height; y++) {
                for (int x = 0; x < field->width; x++) {
                    if (field->battlefield[y][x] != TERRAIN_HALF_COVER && field->battlefield[y][x] != TERRAIN_FULL_COVER) {
                        continue;
                    }
                    // Check if that spot is occupied another unit
                    bool enemy_on_cover = false;
                    for (int i = 0; i < field->unit_count; i++) {
                        Unit* other = field->units[i];
                        if (!other || other == unit || other->hp <= 0) {
                            continue;
                        }
                        if (other->x == x && other->y == y) { 
                            enemy_on_cover = true; 
                            break; 
                        }
                    }
                    if (enemy_on_cover) {
                        continue;
                    }

                    // Check if the cover can be reached in movement
                    int dx = unit->x - x;
                    int dy = unit->y - y;
                    int dist_to_cover = abs(dx) + abs(dy);
                    if (dist_to_cover > unit->movement) {
                        continue;
                    }
                    // Compute distance from cover to enemy
                    int dce = abs(closest_enemy->x - x) + abs(closest_enemy->y - y);
                    if (dce < current_dist_to_enemy) {
                        // Choose the cover with minimal distance to enemy
                        if (dce < best_cover_dist) {
                            best_cover_dist = dce;
                            best_x = x;
                        }
                    }
                }
            }

            // Return if the spot is valid
            if (best_x != -1) {
                return true;
            }
            return false;


        case 'd': // Dash
            // Select a target and run (maybe)
            target = select_enemy_by_preference(unit, field, 100, &strat->strategic);
            return (target != NULL);

        case 'r': // Retreat
            target = find_closest_enemy(unit, field);
            if (!target) {
                return false;
            }
            int dist = get_distance_squared(unit, target);
            // Retreat if enemy is within danger_range
            return (dist <= unit->danger_range_sq);

        default:
            return false;
    }
}

// Unit turn
void unit_turn(Unit* unit, Battlefield* field, StrategicOrder* order) {
    // Units follow an order of operations
    target = NULL;
    bool acted = false;
    int action_taken = -1;

    // Use normal strategic orders if none are given
    StrategicOrder* effective_order = order;
    if (effective_order == NULL) {
        effective_order = &unit->strategic_order;
    }

    FactionStrategy* strat;
    if (unit->faction == FACTION_REPUBLIC) {
        strat = &republic_strategy;
    } else {
        strat = &separatist_strategy;
    }

    // Multiply global by unit-specific modifier
    float final_explosive_threshold = strat->tactical.use_explosive_threshold * unit->effective_explosive_threshold;
    float final_ally_proximity = strat->strategic.ally_proximity * unit->effective_ally_proximity;
    float final_cover_preference = strat->tactical.cover_preference * unit->effective_cover_preference;
    float final_retreat_hp_ratio = strat->tactical.retreat_hp_ratio * unit->effective_retreat_hp_ratio;

    // Adjust values to valid ranges
    if (final_explosive_threshold < 1.0f) {
        final_explosive_threshold = 1.0f;
    }
    if (final_explosive_threshold > 4.0f) {
        final_explosive_threshold = 4.0f;
    }
    if (final_ally_proximity < 2.0f) {
        final_ally_proximity = 2.0f;
    }
    if (final_ally_proximity > 8.0f) {
        final_ally_proximity = 8.0f;
    }
    if (final_cover_preference < 0.0f) {
        final_cover_preference = 0.0f;
    }
    if (final_cover_preference > 1.0f) {
        final_cover_preference = 1.0f;
    }
    if (final_retreat_hp_ratio < 0.1f) {
        final_retreat_hp_ratio = 0.1f;
    }
    if (final_retreat_hp_ratio > 0.6f) {
        final_retreat_hp_ratio = 0.6f;
    }

    // Do strategic orders, if available
    // Move away from the nearest enemy
    if (effective_order && effective_order->type == ORDER_RETREAT) {
        Unit* enemy = find_closest_enemy(unit, field);
        if (enemy) {
            execute_retreat(unit, enemy, field);
            acted = true;
            action_taken = ACTION_RETREAT;
        } else {
            // If there is no enemy, move toward the retreat target
            if (effective_order->type == ORDER_MOVE_TO || effective_order->type == ORDER_RETREAT) {
                execute_move(unit, field, effective_order->target_x, effective_order->target_y);
                acted = true;
                action_taken = ACTION_RETREAT;
            }
        }
        // Mark as acted
        goto action_done;
    }

    // Move toward the target point
    if (effective_order && effective_order->type == ORDER_MOVE_TO) {
        execute_move(unit, field, effective_order->target_x, effective_order->target_y);
        acted = true;
        action_taken = ACTION_DASH;
        goto action_done;
    }

    // Do normal actions
    // 1. Emergency rally (officer or heavily pinned)
    if ((unit->type == UNIT_CLONE_OFFICER || unit->type == UNIT_DROID_OFFICER) && unit->pin_markers == 0) {
        acted = execute_rally(unit, field);
        if (acted) { action_taken = ACTION_RALLY; goto action_done; }
    }
    if (unit->pin_markers >= 3) {
        acted = execute_rally(unit, field);
        if (acted) { action_taken = ACTION_RALLY; goto action_done; }
    }

    // 2. Explosive (if has explosives and enemy cluster meets threshold)
    if (!acted && has_explosives(unit)) {
        if (evaluate_enemies(unit, field, 'e', strat, final_explosive_threshold)) {
            acted = execute_explosive(unit, target, false, field);
            if (acted) { action_taken = ACTION_EXPLOSIVE; goto action_done; }
        }
    }

    // 3. Fire (if enemy in range)
    int max_range = unit->max_range;
    if (max_range > 0) {
        Unit* fire_target = NULL;
        // Check focus fire order first
        if (effective_order && effective_order->type == ORDER_FOCUS_FIRE) {
            if (unit->focus_target && (get_distance_squared(unit, unit->focus_target) <= max_range * max_range)) {
                fire_target = unit->focus_target;
            }
        }
        if (!fire_target) {
            if (evaluate_enemies(unit, field, 'f', strat, 0.0f)) {
                fire_target = target;
            }
        }
        if (fire_target) {
            acted = execute_fire(unit, fire_target, false, field);
            if (acted) { action_taken = ACTION_FIRE; goto action_done; }
        }
    }

    // 4. Advance & Fire (if enemy beyond preferred range but within max range + half movement)
    if (!acted && !(effective_order && effective_order->type == ORDER_DEFEND)) {
        Unit* enemy = find_closest_enemy(unit, field);
        if (enemy) {
            int dist_sq = get_distance_squared(unit, enemy);
            int max_advance_dist = max_range + unit->half_movement;
            if (dist_sq > unit->preferred_range_sq && dist_sq <= max_advance_dist * max_advance_dist) {
                move_toward_target(unit, enemy, unit->half_movement, field);
                acted = execute_fire(unit, enemy, true, field);
                if (acted) { action_taken = ACTION_ADVANCE_FIRE; goto action_done; }
            }
        }
    }

    // 5. Retreat (if enemy is within danger range)
    if (!acted && !(effective_order && effective_order->type == ORDER_ADVANCE)) {
        if (evaluate_enemies(unit, field, 'r', strat, 0.0f)) {
            if (unit->hp < unit->max_hp * 0.3f || unit->pin_markers >= 2) {
                execute_retreat(unit, target, field);
                acted = true;
                action_taken = ACTION_RETREAT;
                goto action_done;
            }
        }
    }

    // 6. Dash (move toward enemy if no target in range)
    if (!acted && !(effective_order && effective_order->type == ORDER_DEFEND)) {
        bool moved_formation = false;
        if (should_use_formation(unit)) {
            if (effective_order && (effective_order->type == ORDER_ADVANCE || effective_order->type == ORDER_MOVE_TO
                || effective_order->type == ORDER_FLANK_LEFT || effective_order->type == ORDER_FLANK_RIGHT
                || effective_order->type == ORDER_BREAKTHROUGH || effective_order->type == ORDER_FALLBACK)) {
                if (!unit->squad_moved) {
                    int move_amount = unit->half_movement;
                    if (move_amount < 1) move_amount = 1;
                    execute_squad_move(unit, field, effective_order->target_x, effective_order->target_y, move_amount);
                    int sid = unit->squad_id;
                    for (int i = 0; i < field->unit_count; i++) {
                        Unit* other = field->units[i];
                        if (other != NULL && other->hp > 0 && other->squad_id == sid) {
                            other->squad_moved = true;
                        }
                    }
                    moved_formation = true;
                    acted = true;
                    action_taken = ACTION_DASH;
                    goto action_done;
                }
            }
        }
        if (!moved_formation && evaluate_enemies(unit, field, 'd', strat, 0.0f)) {
            if (target && unit->movement > 0 && !unit->is_immobilized) {
                execute_dash(unit, target, field);
                clamp_unit_position(unit, field);
                acted = true;
                action_taken = ACTION_DASH;
                goto action_done;
            }
        }
    }

    // 7. Move to cover (only if defending and low HP/pins)
    if (!acted) {
        if (effective_order && effective_order->type == ORDER_DEFEND) {
            if (unit->hp < unit->max_hp * 0.4f || unit->pin_markers >= 2) {
                execute_move_to_cover(unit, field);
                acted = true;
                action_taken = ACTION_COVER;
                goto action_done;
            }
        }
        // move to cover if low HP
        if (!acted && (unit->hp < unit->max_hp * 0.3f || unit->pin_markers >= 3)) {
            if (evaluate_enemies(unit, field, 'c', strat, 0.0f)) {
                execute_move_to_cover(unit, field);
                acted = true;
                action_taken = ACTION_COVER;
                goto action_done;
            }
        }
    }

    // 8. If still not acted, rally (if pinned) or do nothing
    if (!acted) {
        if (unit->pin_markers > 0) {
            execute_rally(unit, field);
            action_taken = ACTION_RALLY;
        } else {
            action_taken = -1;
        }
        acted = true;
    }

    action_done:

    if (turn_based) {
        const char* action_names[] = {
            "Fire", "Advance+Fire", "Explosive",
            "Rally", "Dash", "Retreat", "Cover", "Advance+Cover"
        };
        char msg[128];
        const char* action_name;
        if (action_taken >= 0 && action_taken < 8) {
            action_name = action_names[action_taken];
        } else {
            action_name = "Nothing";
        }
        int turn = get_turn_count();
        const char* type_name = get_unit_type_name(unit->type);
        snprintf(msg, sizeof(msg), "Turn %d: [%s] Unit %d %s at (%d,%d)\n",
                turn, type_name, unit->id, action_name, unit->x, unit->y);
        strncat(battle_action_log, msg, sizeof(battle_action_log) - strlen(battle_action_log) - 1);
    }
    if (turn_based && action_taken >= 0 && target) {
        char target_msg[64];
        snprintf(target_msg, sizeof(target_msg), "  -> Target: %s (HP:%d)\n",
                get_unit_type_name(target->type), target->hp);
        strncat(battle_action_log, target_msg, sizeof(battle_action_log) - strlen(battle_action_log) - 1);
    }

    target = NULL;

    // Tracking
    if (track_actions) {
        if (unit->actions_taken == 0 && !unit->has_acted) {
            units_that_acted++;
        }
        unit->actions_taken++;
        total_actions_across_units++;
    }

    if (track_actions && action_taken >= 0) {
        switch(action_taken) {
            case 0: action_counts.fire++; break;
            case 1: action_counts.advance_fire++; break;
            case 2: action_counts.explosive++; break;
            case 3: action_counts.rally++; break;
            case 4: action_counts.dash++; break;
            case 5: action_counts.retreat++; break;
            case 6: action_counts.cover++; break;
            case 7: action_counts.advance_cover++; break;
        }
        action_counts.total++;
    }
}

// Compare two units based on preference digit (1=HP, 2=Pins, 3=Value, 4=Armor)
int compare_enemies_by_preference(const Unit* a, const Unit* b, int preference_digit) {
    switch(preference_digit) {
        case 1: // Lowest HP and not armored
            if (a->hp < b->hp && !a->is_armor) {
                return -1;
            }
            if (a->hp > b->hp && !b->is_armor) {
                return 1;
            }
            return 0;
        case 2: // Highest Pins
            if (a->pin_markers > b->pin_markers && !a->is_armor) {
                return -1;
            }
            if (a->pin_markers < b->pin_markers && !b->is_armor) {
                return 1;
            }
            return 0;
        case 3: // Highest Point Value
            if (a->point_value > b->point_value && !a->is_armor) {
                return -1;
            }
            if (a->point_value < b->point_value && !b->is_armor) {
                return 1;
            }
            return 0;
        case 4: // Highest Armor Class
            if (a->armor_class > b->armor_class && !a->is_armor) {
                return -1;
            }
            if (a->armor_class < b->armor_class && !b->is_armor) {
                return 1;
            }
            return 0;
        default: return 0;
    }
}

// Select the best enemy within a given maximum range using the unit's attack_order
Unit* select_enemy_by_preference(Unit* unit, Battlefield* field, int max_range, StrategicGenome* strat) {
    if (!unit) return NULL;

    // Stage 1: gather candidates and compute rough scores (no cover)
    typedef struct { Unit* u; float rough_score; } Candidate;
    Candidate candidates[MAX_UNITS];
    int count = 0;
    float max_dist = (float)unit->max_range;
    if (max_dist < 1.0f) max_dist = 1.0f;

    for (int i = 0; i < field->unit_count; i++) {
        Unit* other = field->units[i];
        if (!other || other == unit || other->hp <= 0) continue;
        if (other->faction == unit->faction) continue;
        int dist_sq = get_distance_squared(unit, other);
        if (dist_sq > max_range * max_range) continue;
        // Rough score: HP + distance threat (no cover)
        float hp_score = 1.0f - ((float)other->hp / other->max_hp);
        float dist = sqrt((float)dist_sq);
        float dist_threat = 1.0f - (dist / max_dist);
        if (dist_threat < 0.0f) dist_threat = 0.0f;
        float rough = (1.0f - strat->threat_weight) * hp_score + strat->threat_weight * dist_threat;
        candidates[count].u = other;
        candidates[count].rough_score = rough;
        count++;
        if (count >= MAX_UNITS) break;
    }
    if (count == 0) return NULL;

    // Stage 2: select top N candidates based on rough score
    // select top 5, or all if less than 5.
    int top_n = (count < 5) ? count : 5;
    // selection sort to get top N (small N)
    for (int i = 0; i < top_n; i++) {
        int best_idx = i;
        for (int j = i+1; j < count; j++) {
            if (candidates[j].rough_score > candidates[best_idx].rough_score)
                best_idx = j;
        }
        if (best_idx != i) {
            Candidate tmp = candidates[i];
            candidates[i] = candidates[best_idx];
            candidates[best_idx] = tmp;
        }
    }

    // Stage 3: compute refined score with cover for top n
    float best_score = -9999.0f;
    Unit* best = candidates[0].u;
    for (int i = 0; i < top_n; i++) {
        Unit* candidate = candidates[i].u;
        // Recompute HP and distance threat
        float hp_score = 1.0f - ((float)candidate->hp / candidate->max_hp);
        float dist = sqrt((float)get_distance_squared(unit, candidate));
        float dist_threat = 1.0f - (dist / max_dist);
        if (dist_threat < 0.0f) dist_threat = 0.0f;
        float base_score = (1.0f - strat->threat_weight) * hp_score + strat->threat_weight * dist_threat;

        int cover_bonus = get_cover_bonus_between(unit, candidate, field);
        float cover_penalty;
        if (cover_bonus == -1) {
            cover_penalty = 1.0f; // penalized
        } else {
            cover_penalty = (float)cover_bonus / 2.0f;
        }
        float cover_modifier = 1.0f - cover_penalty * 0.6f;

        // Pin and value modifiers
        float pin_score = (float)candidate->pin_markers / 5.0f; // max pins ~? 
        float pin_modifier = 1.0f + pin_score * 0.3f;
        float value_modifier = 1.0f + ((float)candidate->point_value / 100.0f) * 0.2f;

        float score = base_score * cover_modifier * pin_modifier * value_modifier;
        if (score > best_score) {
            best_score = score;
            best = candidate;
        }
    }
    return best;
}