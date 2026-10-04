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

// Unit turn
void unit_turn(Unit* unit, Battlefield* field) {
    target = NULL;
    bool acted = false;
    int action_taken = -1;
    bool is_ranged = unit->is_ranged;
    bool moved = false;

    FactionStrategy* strat = (unit->faction == FACTION_REPUBLIC)
                             ? &republic_strategy : &separatist_strategy;

    int max_range = unit->max_range;

    // 1. Emergency rally
    if (unit->pin_markers >= 3) {
        acted = execute_rally(unit);
        if (acted) { action_taken = ACTION_RALLY; goto action_done; }
    }

    // 2. Attack, if an enemy is in range
    if (!acted) {
        Unit* fire_target = select_enemy_by_preference(unit, field, max_range, &strat->strategic);
        if (fire_target) {
            target = fire_target;
            if (is_ranged) {
                acted = execute_fire(unit, fire_target, false, field);
                if (acted) { action_taken = ACTION_FIRE; goto action_done; }
            } else {
                acted = execute_attack(unit, fire_target, false, field);
                if (acted) { action_taken = ACTION_ATTACK; goto action_done; }
            }
        }
    }

    // 3. Advance and attack, if enemy is close enough
    if (!acted) {
        Unit* enemy = find_closest_enemy(unit, field);
        if (enemy) {
            int dist_sq = get_distance_squared(unit, enemy);
            int max_advance = max_range + unit->half_movement;
            if (dist_sq > unit->preferred_range_sq && dist_sq <= max_advance * max_advance) {
                move_toward_target(unit, enemy, unit->half_movement, field);
                moved = true;
                if (is_ranged) {
                    acted = execute_fire(unit, enemy, true, field);
                    if (acted) { action_taken = ACTION_ADVANCE_FIRE; goto action_done; }
                } else {
                    acted = execute_attack(unit, enemy, true, field);
                    if (acted) { action_taken = ACTION_ADVANCE_ATTACK; goto action_done; }
                }
                action_taken = is_ranged ? ACTION_ADVANCE_FIRE : ACTION_ADVANCE_ATTACK;
                goto action_done;
            }
        }
    }

    // 4. Move toward strategic target, or dash toward enemy
    if (!acted && !moved) {
        int tx, ty;
        bool has_target = false;

        if (unit->role == ROLE_FLANK) {
        // Flankers steer around the enemy
        int ecx, ecy;
        get_faction_center(field, enemy_faction(unit->faction), &ecx, &ecy);
        int offset = (int)(strat->strategic.flanking_bias * 8.0f);
        bool go_left = (unit->id % 2 == 0);
        tx = ecx + (go_left ? -offset : offset);
        ty = ecy;
        has_target = true;
        } else {
            // Line units advance toward the enemy center.
            int ecx, ecy;
            get_faction_center(field, enemy_faction(unit->faction), &ecx, &ecy);
            tx = ecx; ty = ecy;
            has_target = true;
        }
        if (has_target) {
            int move_cap = unit->half_movement + (int)((unit->movement - unit->half_movement) * unit->effective_aggression);
            execute_move(unit, field, tx, ty, move_cap);
            acted = true;
            action_taken = ACTION_DASH;
        }
    }

    // 5. Fallback: rally if pinned
    if (!acted) {
        if (unit->pin_markers > 0) {
            execute_rally(unit);
            action_taken = ACTION_RALLY;
        }
        acted = true;
    }

action_done:
    if (turn_based) {
        const char* action_names[] = {
            "Fire", "Advance+Fire",
            "Attack", "Advance+Attack",
            "Rally", "Dash", "Retreat", "Cover", "Advance+Cover"
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
        }
        action_counts.total++;
    }
}

// Select the best enemy within a given maximum range using the unit's attack_order
Unit* select_enemy_by_preference(Unit* unit, Battlefield* field, int max_range, StrategicGenome* strat) {
    if (!unit) return NULL;

    // Stage 1: gather candidates and compute rough scores
    typedef struct { Unit* u; float rough_score; } Candidate;
    Candidate candidates[MAX_UNITS];
    int count = 0;
    float max_dist = (float)max_range;
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