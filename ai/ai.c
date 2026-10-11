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

#define LOCAL_FORCE_RADIUS 5

static void move_with_spacing(Unit* unit, Battlefield* field, int tx, int ty, FactionStrategy* strat);
static int line_turn(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage);
static int flank_turn(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage);
static int ranged_turn(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage);

// Global variable for the target
static Unit* target = NULL;

// Unit turn
void unit_turn(Unit* unit, Battlefield* field) {
    target = NULL;

    FactionStrategy* strat = (unit->faction == FACTION_REPUBLIC)
                             ? &republic_strategy : &separatist_strategy;

    int local_allies  = count_faction_in_radius(unit, field, unit->faction, LOCAL_FORCE_RADIUS) + 1;
    int local_enemies = count_faction_in_radius(unit, field,
                            enemy_faction(unit->faction), LOCAL_FORCE_RADIUS);
    bool should_engage = (local_enemies == 0) ||
        ((float)local_allies >= unit->effective_local_force_ratio * (float)local_enemies);

    int action_taken;
    switch (unit->role) {
        case ROLE_RANGED: action_taken = ranged_turn(unit, field, strat, should_engage); break;
        case ROLE_FLANK:  action_taken = flank_turn(unit, field, strat, should_engage);  break;
        default:          action_taken = line_turn(unit, field, strat, should_engage);   break;
    }

    // --- shared logging block below, unchanged ---
    if (turn_based) {
        // ... existing action_names, snprintf, strncat ...
    }
    if (turn_based && action_taken >= 0 && target) {
        // ... existing target log ...
    }

    target = NULL;

    if (track_actions) {
        if (unit->actions_taken == 0 && !unit->has_acted) units_that_acted++;
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

// Shared helper: move toward (tx, ty) with spacing adjustment
static void move_with_spacing(Unit* unit, Battlefield* field, int tx, int ty, FactionStrategy* strat) {
    Unit* ally = find_closest_ally(unit, field);
    if (ally) {
        int dax = unit->x - ally->x;
        int day = unit->y - ally->y;
        if (fabsf((float)dax) < unit->spacing_x) {
            int push = (int)(unit->spacing_x - fabsf((float)dax));
            int dir = (dax != 0) ? (dax > 0 ? 1 : -1) : ((unit->id % 2 == 0) ? 1 : -1);
            tx += dir * push;
        }
        if (fabsf((float)day) < unit->spacing_y) {
            int push = (int)(unit->spacing_y - fabsf((float)day));
            int dir = (day != 0) ? (day > 0 ? 1 : -1) : ((unit->id % 2 == 0) ? 1 : -1);
            ty += dir * push;
        }
    }

    // Faction-wide multiplier on top of per-unit aggression.
    float effective = unit->effective_aggression * strat->strategic.aggression_bias;
    if (effective > 1.0f) effective = 1.0f;
    if (effective < 0.0f) effective = 0.0f;

    int move_cap = unit->half_movement +
        (int)((unit->movement - unit->half_movement) * effective);
    if (move_cap < unit->half_movement) move_cap = unit->half_movement;
    if (move_cap > unit->movement)      move_cap = unit->movement;

    execute_move(unit, field, tx, ty, move_cap);
}

static int line_turn(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage) {
    if (unit->pin_markers >= 3 && execute_rally(unit)) return ACTION_RALLY;

    // 1. Attack enemy in range
    if (should_engage) {
        Unit* fire_target = select_enemy_by_preference(unit, field, unit->max_range, &strat->strategic);
        if (fire_target) {
            target = fire_target;
            bool hit = unit->is_ranged ? execute_fire(unit, fire_target, false)
                                       : execute_attack(unit, fire_target, false);
            if (hit) return unit->is_ranged ? ACTION_FIRE : ACTION_ATTACK;
        }
    }

    // 2. Advance and attack
    if (should_engage) {
        Unit* enemy = find_closest_enemy(unit, field);
        if (enemy) {
            int dist_sq = get_distance_squared(unit, enemy);
            int max_advance = unit->max_range + unit->half_movement;
            if (dist_sq > unit->preferred_range_sq && dist_sq <= max_advance * max_advance) {
                move_toward_target(unit, enemy, unit->half_movement, field);
                return unit->is_ranged ? ACTION_ADVANCE_FIRE : ACTION_ADVANCE_ATTACK;
            }
        }
    }

    // 3. March toward enemy center (or regroup if outnumbered)
    int tx, ty;
    if (should_engage) {
        get_faction_center(field, enemy_faction(unit->faction), &tx, &ty);
    } else {
        get_faction_center(field, unit->faction, &tx, &ty);
    }
    move_with_spacing(unit, field, tx, ty, strat);
    return ACTION_DASH;
}

static int flank_turn(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage) {
    if (unit->pin_markers >= 3 && execute_rally(unit)) return ACTION_RALLY;

    // 1. Attack opportunistically (but don't chase)
    if (should_engage) {
        Unit* fire_target = select_enemy_by_preference(unit, field, unit->max_range, &strat->strategic);
        if (fire_target) {
            target = fire_target;
            bool hit = unit->is_ranged ? execute_fire(unit, fire_target, false)
                                       : execute_attack(unit, fire_target, false);
            if (hit) return unit->is_ranged ? ACTION_FIRE : ACTION_ATTACK;
        }
    }

    // 2. Keep moving toward the flank position. Note: no advance+attack step,
    //    so the flanker doesn't get sucked into the line.
    int ecx, ecy;
    get_faction_center(field, enemy_faction(unit->faction), &ecx, &ecy);
    int offset = (int)(strat->strategic.flanking_bias * 8.0f);
    int push_dir = (unit->faction == FACTION_REPUBLIC) ? -1 : 1;
    bool go_left = (unit->id % 2 == 0);
    int tx = ecx + (go_left ? -offset : offset);
    int ty = ecy + push_dir * (int)strat->strategic.flank_depth;

    move_with_spacing(unit, field, tx, ty, strat);
    return ACTION_DASH;
}

static int ranged_turn(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage) {
    if (unit->pin_markers >= 3 && execute_rally(unit)) return ACTION_RALLY;

    // 1. Survival check
    Unit* threat = find_closest_enemy(unit, field);
    if (threat && !threat->is_ranged) {
        int threat_dist_sq = get_distance_squared(unit, threat);
        int kite_threshold_sq = unit->preferred_range_sq / 4;
        if (kite_threshold_sq < 1) kite_threshold_sq = 1;

        if (threat_dist_sq < kite_threshold_sq) {
            int dx = (unit->x > threat->x) ? 1 : (unit->x < threat->x) ? -1 : 0;
            int dy = (unit->y > threat->y) ? 1 : (unit->y < threat->y) ? -1 : 0;
            if (dx == 0 && dy == 0) dx = (unit->id % 2) ? 1 : -1;

            int flee_x = unit->x + dx * unit->movement;
            int flee_y = unit->y + dy * unit->movement;
            execute_move(unit, field, flee_x, flee_y, unit->movement);
            return ACTION_DASH;
        }
    }

    // 2. Fire at best target
    if (should_engage) {
        Unit* fire_target = select_enemy_by_preference(unit, field, unit->max_range, &strat->strategic);
        if (fire_target) {
            target = fire_target;
            if (execute_fire(unit, fire_target, false)) return ACTION_FIRE;
        }
    }

    // 3. Reposition toward preferred range if too far
    if (should_engage && threat) {
        int dist_sq = get_distance_squared(unit, threat);
        if (dist_sq > unit->preferred_range_sq) {
            move_toward_target(unit, threat, unit->half_movement, field);
            return ACTION_ADVANCE_FIRE;
        }
        // Inside preferred range but no shot available? Hold position.
        return -1;
    }

    // 4. Regroup if not engaging
    if (!should_engage) {
        int fcx, fcy;
        get_faction_center(field, unit->faction, &fcx, &fcy);
        move_with_spacing(unit, field, fcx, fcy, strat);
        return ACTION_DASH;
    }

    return -1;
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