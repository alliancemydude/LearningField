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
bool evaluate_enemies(Unit* unit, Battlefield* field, char type, FactionStrategy* strat) {
    int max_range = unit->max_range;
    switch(type) {
        case 'a': // Attack
            if (max_range == 0) {
                return false;
            }
            target = select_enemy_by_preference(unit, field, max_range, &strat->strategic);
            return (target != NULL);

        case 'f': // Fire
            if (max_range == 0) {
                return false;
            }
            target = select_enemy_by_preference(unit, field, max_range, &strat->strategic);
            return (target != NULL);

        case 'd': // Dash
            // Select a target and run
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
    float final_ally_proximity = strat->strategic.ally_proximity * unit->effective_ally_proximity;
    float final_retreat_hp_ratio = strat->tactical.retreat_hp_ratio * unit->effective_retreat_hp_ratio;

    // Adjust values to valid ranges
    if (final_ally_proximity < 2.0f) {
        final_ally_proximity = 2.0f;
    }
    if (final_ally_proximity > 8.0f) {
        final_ally_proximity = 8.0f;
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
    // 1. Emergency rally
    if (unit->pin_markers >= 3) {
        acted = execute_rally(unit);
        if (acted) { action_taken = ACTION_RALLY; goto action_done; }
    }

    // 2. Fire or attack, if an enemy is in range
    int max_range = unit->max_range;
    if (max_range > 4) {
        // Fire at range
        Unit* fire_target = NULL;

        // Check focus fire order first
        if (effective_order && effective_order->type == ORDER_FOCUS_FIRE) {
            if (unit->focus_target && (get_distance_squared(unit, unit->focus_target) <= max_range * max_range)) {
                fire_target = unit->focus_target;
            }
        }
        if (!fire_target) {
            if (evaluate_enemies(unit, field, 'f', strat)) {
                fire_target = target;
            }
        }
        if (fire_target) {
            acted = execute_fire(unit, fire_target, false, field);
            if (acted) { action_taken = ACTION_FIRE; goto action_done; }
        }

    } else {
        // Melee attack
        Unit* fire_target = NULL;

        if (!fire_target) {
            if (evaluate_enemies(unit, field, 'a', strat)) {
                fire_target = target;
            }
        }

        if (fire_target) {
            acted = execute_attack(unit, fire_target, false, field);
            if (acted) { action_taken = ACTION_ATTACK; goto action_done; }
        }
    }

    // 3. Advance & Fire (if enemy beyond preferred range but within max range + half movement)
    if (!acted && !(effective_order && effective_order->type == ORDER_DEFEND)) {
        Unit* enemy = find_closest_enemy(unit, field);
        if (enemy) {
            int dist_sq = get_distance_squared(unit, enemy);
            int max_advance_dist = max_range + unit->half_movement;
            if (dist_sq > unit->preferred_range_sq && dist_sq <= max_advance_dist * max_advance_dist) {
                move_toward_target(unit, enemy, unit->half_movement, field);
                if (max_range > 4) {
                    acted = execute_fire(unit, enemy, true, field);
                    if (acted) { action_taken = ACTION_ADVANCE_FIRE; goto action_done; }
                } else {
                    acted = execute_attack(unit, enemy, true, field);
                    if (acted) { action_taken = ACTION_ADVANCE_ATTACK; goto action_done; }
                }
                
            }
        }
    }

    // 4. Retreat (if enemy is within danger range)
    if (!acted && !(effective_order && effective_order->type == ORDER_ADVANCE)) {
        if (evaluate_enemies(unit, field, 'r', strat)) {
            if (unit->hp < unit->max_hp * 0.3f || unit->pin_markers >= 2) {
                execute_retreat(unit, target, field);
                acted = true;
                action_taken = ACTION_RETREAT;
                goto action_done;
            }
        }
    }

    // 5. Dash (move toward enemy if no target in range)
    if (!acted && !(effective_order && effective_order->type == ORDER_DEFEND)) {
        bool moved_formation = false;
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
        if (!moved_formation && evaluate_enemies(unit, field, 'd', strat)) {
            if (target && unit->movement > 0 && !unit->is_immobilized) {
                execute_dash(unit, target, field);
                clamp_unit_position(unit, field);
                acted = true;
                action_taken = ACTION_DASH;
                goto action_done;
            }
        }
    }

    // 6. If still not acted, rally (if pinned) or do nothing
    if (!acted) {
        if (unit->pin_markers > 0) {
            execute_rally(unit);
            action_taken = ACTION_RALLY;
        } else {
            action_taken = -1;
        }
        acted = true;
    }

    action_done:

    if (turn_based) {
        const char* action_names[] = {
            "Fire",             // 0 ACTION_FIRE
            "Advance+Fire",     // 1 ACTION_ADVANCE_FIRE
            "Attack",           // 2 ACTION_ATTACK
            "Advance+Attack",   // 3 ACTION_ADVANCE_ATTACK
            "Rally",            // 4 ACTION_RALLY
            "Dash",             // 5 ACTION_DASH
            "Retreat",          // 6 ACTION_RETREAT
            "Cover",            // 7 ACTION_COVER
            "Advance+Cover"     // 8 ACTION_ADVANCE_COVER
        };
        char msg[128];
        const char* action_name;
        if (action_taken >= 0 && action_taken < 9) {
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
            case 2: action_counts.attack++; break;
            case 3: action_counts.advance_attack++; break;
            case 4: action_counts.rally++; break;
            case 5: action_counts.dash++; break;
            case 6: action_counts.retreat++; break;
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

    // Stage 1: gather candidates and compute rough scores
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
        // Rough score: HP + distance threat
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

    // Stage 3: compute refined score
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

        // Pin and value modifiers
        float pin_score = (float)candidate->pin_markers / 5.0f; // max pins ~? 
        float pin_modifier = 1.0f + pin_score * 0.3f;
        float value_modifier = 1.0f + ((float)candidate->point_value / 100.0f) * 0.2f;

        float score = base_score * pin_modifier * value_modifier;
        if (score > best_score) {
            best_score = score;
            best = candidate;
        }
    }
    return best;
}