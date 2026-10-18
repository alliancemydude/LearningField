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
static void line_move_phase(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage);
static void flank_move_phase(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage);
static void ranged_move_phase(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage);

// Unit turn, for move and attack
void unit_move_phase(Unit* unit, Battlefield* field) {
    FactionStrategy* strat = (unit->faction == FACTION_GERMAN)
                             ? &german_strategy : &roman_strategy;

    int local_allies  = unit->local_allies + 1;
    int local_enemies = unit->local_enemies;
    bool should_engage = (local_enemies == 0) ||
        ((float)local_allies >= unit->effective_local_force_ratio * (float)local_enemies);

    // Emergency rally consumes the whole turn
    if (unit->pin_markers >= 3) {
        if (execute_rally(unit)) {
            unit->rallied_this_turn = true;
            return;
        }
    }

    // Face threat (existing logic, moved up from unit_turn)
    if (strat->strategic.face_threat_weight > 0.0f) {
        Unit* threat = find_closest_enemy(unit, field);
        if (threat) {
            int angle = get_attack_angle(threat, unit);
            if (angle > 0 && roll(0, 99) < (int)(strat->strategic.face_threat_weight * 100.0f)) {
                int dx = threat->x - unit->x;
                int dy = threat->y - unit->y;
                if (abs(dx) > abs(dy)) {
                    unit->facing = (dx > 0) ? FACE_EAST : FACE_WEST;
                } else {
                    unit->facing = (dy > 0) ? FACE_SOUTH : FACE_NORTH;
                }
            }
        }
    }

    int old_x = unit->x, old_y = unit->y;

    switch (unit->role) {
        case ROLE_RANGED: ranged_move_phase(unit, field, strat, should_engage); break;
        case ROLE_FLANK:  flank_move_phase(unit, field, strat, should_engage);  break;
        default:          line_move_phase(unit, field, strat, should_engage);   break;
    }

    unit->moved_this_turn = (unit->x != old_x || unit->y != old_y);
}

void unit_attack_phase(Unit* unit, Battlefield* field) {
    if (unit->rallied_this_turn) return;

    FactionStrategy* strat = (unit->faction == FACTION_GERMAN)
                             ? &german_strategy : &roman_strategy;
    int local_allies  = unit->local_allies + 1;
    int local_enemies = unit->local_enemies;
    bool should_engage = (local_enemies == 0) ||
        ((float)local_allies >= unit->effective_local_force_ratio * (float)local_enemies);

    if (!should_engage) return;

    Unit* target = select_enemy_by_preference(unit, field, unit->max_range, &strat->strategic);
    if (!target) return;

    bool moved = unit->moved_this_turn;
    if (unit->is_ranged) {
        execute_fire(unit, target, moved);
    } else {
        execute_attack(unit, target, moved);
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

static void line_move_phase(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage) {
    if (!should_engage) {
        int tx, ty;
        get_faction_center(field, unit->faction, &tx, &ty);
        move_with_spacing(unit, field, tx, ty, strat);
        return;
    }

    Unit* enemy = find_closest_enemy(unit, field);
    if (!enemy) return;

    int dist_sq = get_distance_squared(unit, enemy);
    if (dist_sq <= unit->preferred_range_sq) return;   // in range — hold and attack

    int enemy_dir = (unit->faction == FACTION_GERMAN) ? -1 : 1;
    int standoff = (int)sqrtf((float)unit->preferred_range_sq);
    int target_y = enemy->y - enemy_dir * standoff;

    // Try to advance straight forward (Y only)
    int start_x = unit->x, start_y = unit->y;
    execute_move(unit, field, unit->x, target_y, unit->movement);

    // If completely blocked fall back to a diagonal position
    if (unit->x == start_x && unit->y == start_y) {
        move_toward_target(unit, enemy, unit->movement, field);
    }
}

static void flank_move_phase(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage) {
    // If badly outnumbered, break off the flank and regroup instead of diving in
    if (!should_engage) {
        int fcx, fcy;
        get_faction_center(field, unit->faction, &fcx, &fcy);
        move_with_spacing(unit, field, fcx, fcy, strat);
        return;
    }

    int ecx, ecy;
    get_faction_center(field, enemy_faction(unit->faction), &ecx, &ecy);
    int offset = (int)(strat->strategic.flanking_bias * 8.0f);
    int push_dir = (unit->faction == FACTION_GERMAN) ? -1 : 1;
    bool go_left = (unit->id % 2 == 0);
    int tx = ecx + (go_left ? -offset : offset);
    int ty = ecy + push_dir * (int)strat->strategic.flank_depth;

    move_with_spacing(unit, field, tx, ty, strat);
}

static void ranged_move_phase(Unit* unit, Battlefield* field, FactionStrategy* strat, bool should_engage) {
    Unit* threat = find_closest_enemy(unit, field);
    if (!threat) return;

    int threat_dist_sq = get_distance_squared(unit, threat);
    int kite_threshold_sq = unit->preferred_range_sq / 4;
    if (kite_threshold_sq < 1) kite_threshold_sq = 1;

    // Too close: flee
    if (threat_dist_sq < kite_threshold_sq) {
        int dx = (unit->x > threat->x) ? 1 : (unit->x < threat->x) ? -1 : 0;
        int dy = (unit->y > threat->y) ? 1 : (unit->y < threat->y) ? -1 : 0;
        if (dx == 0 && dy == 0) dx = (unit->id % 2) ? 1 : -1;

        int flee_x = unit->x + dx * unit->movement;
        int flee_y = unit->y + dy * unit->movement;
        execute_move(unit, field, flee_x, flee_y, unit->movement);
        return;
    }

    // Too far: close
    if (should_engage && threat_dist_sq > unit->preferred_range_sq) {
        move_toward_target(unit, threat, unit->movement, field);
        return;
    }

    // Not engaging: regroup
    if (!should_engage) {
        int fcx, fcy;
        get_faction_center(field, unit->faction, &fcx, &fcy);
        move_with_spacing(unit, field, fcx, fcy, strat);
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

        // Bonus for targets we can hit from the flank or rear.
        int angle = get_attack_angle(unit, candidate);
        float angle_bonus = 1.0f;
        if (angle == 1) angle_bonus += strat->flank_targeting_weight * 0.3f;
        if (angle == 2) angle_bonus += strat->flank_targeting_weight * 0.6f;

        float score = base_score * angle_bonus * pin_modifier * value_modifier;
        if (score > best_score) {
            best_score = score;
            best = candidate;
        }
    }
    return best;
}