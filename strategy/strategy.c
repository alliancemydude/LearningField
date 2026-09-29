#include "strategy.h"
#include "battlefield.h"
#include "unit.h"
#include "queries.h"
#include "utils.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Set the current weights
FactionStrategy republic_strategy = {
    .tactical = {
        .advance_distance = 8.0f,
        .defend_distance = 9.0f,
        .flanking_bias = 0.51f,
        .use_explosive_threshold = 1.92f,
        .retreat_hp_ratio = 0.25f,
        .cohesion_threshold = 7.99f
    },
    .strategic = {
        .aggression_bias = 1.21f,
        .aggression_curve = 4.52f,
        .target_priority = 0.42f,
        .ally_proximity = 7.35f,
        .threat_weight = 0.36,
        .focus_fire_tendency = 1.94f,
        .flank_left_weight = 3.02f,
        .flank_right_weight = 8.25f,
        .breakthrough_weight = 1.53f,
        .fallback_weight = 1.32f,
        .edge_avoidance_weight = 0.6f
    },
    .deployment = {
        .deploy_horizontal_spread = 10.0f,
        .deploy_vertical_offset = 10.0f,
        .deploy_flanking_bias = 3.12f
    }
};

FactionStrategy separatist_strategy = {
    .tactical = {
        .advance_distance = 12.86f,
        .defend_distance = 9.61f,
        .flanking_bias = 0.77f,
        .use_explosive_threshold = 1.79f,
        .retreat_hp_ratio = 0.21f,
        .cohesion_threshold = 4.72f
    },
    .strategic = {
        .aggression_bias = 1.41f,
        .aggression_curve = 1.82f,
        .target_priority = 0.26f,
        .ally_proximity = 4.06f,
        .threat_weight = 0.47,
        .focus_fire_tendency = 1.61f,
        .flank_left_weight = 8.88f,
        .flank_right_weight = 8.11f,
        .breakthrough_weight = 2.22f,
        .fallback_weight = 1.76f,
        .edge_avoidance_weight = 0.6f
    },
    .deployment = {
        .deploy_horizontal_spread = 10.0f,
        .deploy_vertical_offset = 10.0f,
        .deploy_flanking_bias = 1.45f
    }
};

// Helper
static void get_faction_center(Battlefield* field, Faction faction, int* cx, int* cy) {
    // Calculate the general center of enemy forces. 

    int count = 0;
    int sum_x = 0, sum_y = 0;

    // Loop through all units, summing their position
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (u->hp <= 0) {
            continue;
        }
        if (u->faction != faction) {
            continue;
        }
        sum_x += u->x;
        sum_y += u->y;
        count++;
    }

    if (count > 0) {
        // Calculate the mean position of all (living) units
        *cx = sum_x / count;
        *cy = sum_y / count;
    } else {
        *cx = -1;
        *cy = -1;
    }

}

// Compute a threat score for an enemy unit
static float compute_threat_score(Unit* unit, Unit* enemy) {
    if (unit == NULL || enemy == NULL) {
        return 0.0f;
    }

    // Distance threat (closer enemies are given a higher threat score)
    float max_dist = (float)unit->max_range;
    if (max_dist < 1.0f) {
        max_dist = 1.0f;
    }
    float dist = sqrt((float)get_distance_squared(unit, enemy));
    float dist_threat = 1.0f - (dist / max_dist);
    if (dist_threat < 0.0f) {
        dist_threat = 0.0f;
    }

    float hp_deficit = 1.0f - ((float)enemy->hp / enemy->max_hp);
    // total threat score is distance threat + HP deficit
    float threat = (dist_threat * 0.6f) + (hp_deficit * 0.4f);
    return threat;
}

// Mid-combat updates to orders
void update_strategic_orders(Battlefield* field) {
    if (field->unit_count == 0) {
        return;
    }
    
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (u == NULL || u->hp <= 0) {
            continue;
        }
        if (u->squad_id == -1) {
            continue;
        }
        if (!should_use_formation(u)) {
            continue;  // only formation units
        }

        // Get the focus fire tendency from the correct faction
        float focus_tendency;
        if (u->faction == FACTION_REPUBLIC) {
            focus_tendency = republic_strategy.strategic.focus_fire_tendency;
        } else {
            focus_tendency = separatist_strategy.strategic.focus_fire_tendency;
        }


        if (focus_tendency < 0.01f) {
            continue;  // skip this squad
        }

        // Gather all living members of this squad that have weapons
        Unit* squad_members[MAX_UNITS];
        int squad_count = 0;
        for (int k = 0; k < field->unit_count; k++) {
            Unit* member = field->units[k];
            if (member == NULL || member->hp <= 0) {
                continue; // If this unit is invalid, skip it
            }
            if (member->faction != u->faction) {
                continue; // Skip enemies
            }
            if (member->squad_id == u->squad_id && should_use_formation(member)) {
                squad_members[squad_count++] = member; // Group unit squads
            }
        }

        // Don't make squads of 2 or less
        if (squad_count < 2) {
            continue;
        }

        // Find the best focus target among all enemies within range of at least 2 squad members
        Unit* best_target = NULL;
        float best_score = -9999.0f;

        // Loop over all enemy units
        for (int e = 0; e < field->unit_count; e++) {
            Unit* enemy = field->units[e];
            if (enemy == NULL || enemy->hp <= 0) {
                continue; // If this unit is invalid, skip it
            }
            if (enemy->faction == u->faction) {
                continue; // Skip enemies
            }

            // Count how many squad members can reach this enemy
            int reachable_count = 0;
            float total_threat = 0.0f;
            for (int s = 0; s < squad_count; s++) {
                Unit* member = squad_members[s];
                int dist_sq = get_distance_squared(member, enemy);
                if (dist_sq <= member->max_range * member->max_range) {
                    reachable_count++;
                    total_threat += compute_threat_score(member, enemy);
                }
            }

            if (reachable_count < 2) {
                continue;  // need at least 2 to focus fire
            }

            // Compute a score: average threat * (reachable_count / squad_count) * focus_tendency
            float avg_threat = total_threat / reachable_count;
            float coverage = (float)reachable_count / squad_count;
            float score = avg_threat * coverage * (1.0f + focus_tendency * coverage);
            if (score > best_score) {
                best_score = score;
                best_target = enemy;
            }
        }

        // If we found a target, assign ORDER_FOCUS_FIRE to all squad members that can reach it
        if (best_target != NULL) {
            for (int s = 0; s < squad_count; s++) {
                Unit* member = squad_members[s];
                int dist_sq = get_distance_squared(member, best_target);
                if (dist_sq <= member->max_range * member->max_range) {
                    member->strategic_order.type = ORDER_FOCUS_FIRE;
                    member->strategic_order.target_x = best_target->x;
                    member->strategic_order.target_y = best_target->y;
                    member->focus_target = best_target;
                }
            }
        }
    }

    // Apply formation offsets to advance orders
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];

        if (u->hp <= 0) {
            continue;
        }
        if (u->strategic_order.type != ORDER_ADVANCE) {
            continue;
        }
        if (!should_use_formation(u)) {
            continue;
        }

        // Gather all living squad members with ORDER_ADVANCE
        Unit* squad_members[MAX_UNITS];
        int squad_count = 0;
        for (int k = 0; k < field->unit_count; k++) {
            Unit* member = field->units[k];
            if (member->hp > 0 && member->squad_id == u->squad_id && member->strategic_order.type == ORDER_ADVANCE) {
                squad_members[squad_count++] = member;
            }
        }

        if (squad_count < 2) {
            continue;
        }

        // Compute average target position (centre)
        int centre_x = 0, centre_y = 0;
        for (int k = 0; k < squad_count; k++) {
            centre_x += squad_members[k]->strategic_order.target_x;
            centre_y += squad_members[k]->strategic_order.target_y;
        }
        centre_x /= squad_count;
        centre_y /= squad_count;

        // Compute advance direction from squad centre (position) to target centre
        int pos_x = 0, pos_y = 0;
        for (int k = 0; k < squad_count; k++) {
            pos_x += squad_members[k]->x;
            pos_y += squad_members[k]->y;
        }
        pos_x /= squad_count;
        pos_y /= squad_count;

        int dx = centre_x - pos_x;
        int dy = centre_y - pos_y;

        // Determine perpendicular direction
        int perp_x, perp_y;
        if (abs(dx) > abs(dy)) {
            // Movement mostly horizontal, spread vertically (Y)
            perp_x = 0;
            perp_y = 1;
        } else {
            // Movement mostly vertical, spread horizontally (X)
            perp_x = 1;
            perp_y = 0;
        }

        // Apply offsets
        int spacing = 2;
        int half = (squad_count - 1) / 2;
        for (int k = 0; k < squad_count; k++) {
            Unit* member = squad_members[k];
            int offset = (member->formation_index - half) * spacing;
            int tx = centre_x + offset * perp_x;
            int ty = centre_y + offset * perp_y;
            // Clamp to battlefield
            if (tx < 0) tx = 0;
            if (tx >= field->width) tx = field->width - 1;
            if (ty < 0) ty = 0;
            if (ty >= field->height) ty = field->height - 1;
            member->strategic_order.target_x = tx;
            member->strategic_order.target_y = ty;
        }
    }

    // Apply cohesion to all formation squads
    bool squad_cohesion_done[MAX_UNITS] = {false};
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (u->hp <= 0 || u->squad_id == -1) {
            continue;
        }
        if (!should_use_formation(u)) {
            continue;
        }
        int sid = u->squad_id;
        if (!squad_cohesion_done[sid]) {
            squad_cohesion_done[sid] = true;
            FactionStrategy* strat;
            if (u->faction == FACTION_REPUBLIC) {
                strat = &republic_strategy;
            } else {
                strat = &separatist_strategy;
            }
            apply_squad_cohesion(u, field, strat);
        }
    }
}

void assign_faction_orders(Battlefield* field, Faction faction) {
    int enemy_cx, enemy_cy;
    get_faction_center(field, (faction == FACTION_REPUBLIC) ? FACTION_SEPARATIST : FACTION_REPUBLIC, &enemy_cx, &enemy_cy);
    int our_cx, our_cy;
    get_faction_center(field, faction, &our_cx, &our_cy);
    float dist = sqrtf((enemy_cx - our_cx)*(enemy_cx - our_cx) + (enemy_cy - our_cy)*(enemy_cy - our_cy));

    FactionStrategy* strat = (faction == FACTION_REPUBLIC) ? &republic_strategy : &separatist_strategy;

    // Apply map modifiers
    float adjusted_aggression = strat->strategic.aggression_bias;
    float adjusted_advance = strat->tactical.advance_distance;
    float advance_threshold = adjusted_advance * adjusted_aggression;

    // Gather formation units and group by squad
    typedef struct {
        int squad_id;
        Unit* units[MAX_UNITS];
        int count;
        int centre_x;
        int centre_y;
        float dist_to_enemy;
    } SquadGroup;
    SquadGroup groups[MAX_UNITS];
    int group_count = 0;

    // First pass: find all formation units and assign to groups
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (u->hp <= 0 || u->faction != faction) continue;
        if (!should_use_formation(u)) continue;
        int sid = u->squad_id;
        if (sid == -1) continue;
        // Find or create group
        int gidx = -1;
        for (int g = 0; g < group_count; g++) {
            if (groups[g].squad_id == sid) {
                gidx = g;
                break;
            }
        }
        if (gidx == -1) {
            gidx = group_count++;
            groups[gidx].squad_id = sid;
            groups[gidx].count = 0;
        }
        groups[gidx].units[groups[gidx].count++] = u;
    }

    // Handle units not in squads
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (u->hp <= 0 || u->faction != faction) continue;
        // Skip units that already have an order
        if (u->strategic_order.type != ORDER_NONE) continue;

        if (dist > advance_threshold) {
            u->strategic_order.type = ORDER_ADVANCE;
            u->strategic_order.target_x = enemy_cx;
            u->strategic_order.target_y = enemy_cy;
        } else {
            u->strategic_order.type = ORDER_DEFEND;
            u->strategic_order.target_x = our_cx;
            u->strategic_order.target_y = our_cy;
        }
    }

    // If no squads, fallback to individual unit handling
    if (group_count == 0) {
        return;
    }

    // Compute centre and distance for each squad
    for (int g = 0; g < group_count; g++) {
        SquadGroup* sg = &groups[g];
        int sum_x = 0, sum_y = 0;
        for (int i = 0; i < sg->count; i++) {
            sum_x += sg->units[i]->x;
            sum_y += sg->units[i]->y;
        }
        sg->centre_x = sum_x / sg->count;
        sg->centre_y = sum_y / sg->count;
        int dx = sg->centre_x - enemy_cx;
        int dy = sg->centre_y - enemy_cy;
        sg->dist_to_enemy = sqrtf(dx*dx + dy*dy);
    }

    // Sort squads by distance to enemy centre (closest first)
    for (int i = 0; i < group_count - 1; i++) {
        for (int j = i + 1; j < group_count; j++) {
            if (groups[i].dist_to_enemy > groups[j].dist_to_enemy) {
                SquadGroup tmp = groups[i];
                groups[i] = groups[j];
                groups[j] = tmp;
            }
        }
    }

    // SMALL GROUP HANDLING: if total formation units < 4, all get same order
    int total_formation_units = 0;
    for (int g = 0; g < group_count; g++) total_formation_units += groups[g].count;
    if (total_formation_units < 4) {
        StrategicOrderType order = (dist > advance_threshold) ? ORDER_ADVANCE : ORDER_DEFEND;
        int target_x = (order == ORDER_ADVANCE) ? enemy_cx : our_cx;
        int target_y = (order == ORDER_ADVANCE) ? enemy_cy : our_cy;
        for (int g = 0; g < group_count; g++) {
            for (int i = 0; i < groups[g].count; i++) {
                Unit* u = groups[g].units[i];
                u->strategic_order.type = order;
                u->strategic_order.target_x = target_x;
                u->strategic_order.target_y = target_y;
            }
        }
        return;
    }

    // LARGE GROUP: assign roles to squads
    int assault_count = group_count * 0.3;
    int support_count = group_count * 0.4;
    int flank_count = group_count - assault_count - support_count;

    int idx = 0;
    // Assault squads (closest)
    for (int i = 0; i < assault_count && idx < group_count; i++, idx++) {
        SquadGroup* sg = &groups[idx];
        for (int j = 0; j < sg->count; j++) {
            Unit* u = sg->units[j];
            u->strategic_order.type = ORDER_BREAKTHROUGH;
            u->strategic_order.target_x = enemy_cx;
            u->strategic_order.target_y = enemy_cy;
        }
    }

    // Support squads (middle)
    for (int i = 0; i < support_count && idx < group_count; i++, idx++) {
        SquadGroup* sg = &groups[idx];
        for (int j = 0; j < sg->count; j++) {
            Unit* u = sg->units[j];
            u->strategic_order.type = ORDER_SUPPRESS;
            u->strategic_order.target_x = enemy_cx;
            u->strategic_order.target_y = enemy_cy;
        }
    }

    // Flank squads (farthest)
    bool alternate = true;
    for (int i = 0; i < flank_count && idx < group_count; i++, idx++) {
        SquadGroup* sg = &groups[idx];
        for (int j = 0; j < sg->count; j++) {
            Unit* u = sg->units[j];
            if (alternate) {
                u->strategic_order.type = ORDER_FLANK_LEFT;
                u->strategic_order.target_x = enemy_cx - 10;
                u->strategic_order.target_y = enemy_cy;
            } else {
                u->strategic_order.type = ORDER_FLANK_RIGHT;
                u->strategic_order.target_x = enemy_cx + 10;
                u->strategic_order.target_y = enemy_cy;
            }
        }
        alternate = !alternate;
    }

    // Keep unit's target coordinates consistent
    float avoid = strat->strategic.edge_avoidance_weight;
    int margin = 3 + (int)((1.0f - avoid) * 7); // 3 to 10
    int x_min = margin;
    int x_max = field->width - 1 - margin;
    int y_min = margin;
    int y_max = field->height - 1 - margin;

    // Only clamp if margins are valid
    if (x_max > x_min && y_max > y_min) {
        for (int i = 0; i < field->unit_count; i++) {
            Unit* u = field->units[i];
            if (u->hp <= 0 || u->faction != faction) continue;
            // Clamp target coordinates
            if (u->strategic_order.target_x < x_min) u->strategic_order.target_x = x_min;
            if (u->strategic_order.target_x > x_max) u->strategic_order.target_x = x_max;
            if (u->strategic_order.target_y < y_min) u->strategic_order.target_y = y_min;
            if (u->strategic_order.target_y > y_max) u->strategic_order.target_y = y_max;
        }
    }
}

void apply_squad_cohesion(Unit* acting_unit, Battlefield* field, FactionStrategy* strat) {

    if (acting_unit == NULL || strat == NULL) return;
    if (!should_use_formation(acting_unit)) return;
    if (acting_unit->squad_id == -1) return;

    float cohesion_threshold = strat->tactical.cohesion_threshold;
    if (cohesion_threshold < 2.0f) cohesion_threshold = 2.0f;
    if (cohesion_threshold > 8.0f) cohesion_threshold = 8.0f;

    // Only the acting unit's squad
    int squad_id = acting_unit->squad_id;

    // First pass: compute squad centre from all squad members (alive)
    int squad_count = 0;
    int sum_x = 0, sum_y = 0;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* member = field->units[i];
        if (member == NULL || member->hp <= 0) continue;
        if (member->squad_id == squad_id) {
            squad_count++;
            sum_x += member->x;
            sum_y += member->y;
        }
    }
    if (squad_count < 2) return; // no need to adjust single unit

    int centre_x = sum_x / squad_count;
    int centre_y = sum_y / squad_count;

    // Second pass: move each squad member if too far from centre
    for (int i = 0; i < field->unit_count; i++) {
        Unit* member = field->units[i];
        if (member == NULL || member->hp <= 0) continue;
        if (member->squad_id != squad_id) continue;

        int dist_to_centre = abs(member->x - centre_x) + abs(member->y - centre_y);
        if (dist_to_centre > cohesion_threshold) {
            int dx = centre_x - member->x;
            int dy = centre_y - member->y;
            int new_x = member->x;
            int new_y = member->y;
            bool moved = false;

            // If we can reach the centre in one step, try to move there directly.
            if (abs(dx) + abs(dy) <= 1) {
                int target_x = centre_x;
                int target_y = centre_y;
                // Check if the target tile is occupied by any other unit
                bool occupied = false;
                for (int k = 0; k < field->unit_count; k++) {
                    Unit* other = field->units[k];
                    if (other == NULL || other == member) continue;
                    if (other->x == target_x && other->y == target_y) {
                        occupied = true;
                        break;
                    }
                }
                if (!occupied) {
                    new_x = target_x;
                    new_y = target_y;
                    moved = true;
                }
            } else {
                // Try moving one step in the X direction first
                if (dx != 0) {
                    int step_x = (dx > 0) ? 1 : -1;
                    int proposed_x = member->x + step_x;
                    // Check occupancy at (proposed_x, member->y)
                    bool occupied = false;
                    for (int k = 0; k < field->unit_count; k++) {
                        Unit* other = field->units[k];
                        if (other == NULL || other == member) continue;
                        if (other->x == proposed_x && other->y == member->y) {
                            occupied = true;
                            break;
                        }
                    }
                    if (!occupied) {
                        new_x = proposed_x;
                        moved = true;
                    }
                }
                // If X move didn't happen, try Y direction
                if (!moved && dy != 0) {
                    int step_y = (dy > 0) ? 1 : -1;
                    int proposed_y = member->y + step_y;
                    // Check occupancy at (member->x, proposed_y)
                    bool occupied = false;
                    for (int k = 0; k < field->unit_count; k++) {
                        Unit* other = field->units[k];
                        if (other == NULL || other == member) continue;
                        if (other->x == member->x && other->y == proposed_y) {
                            occupied = true;
                            break;
                        }
                    }
                    if (!occupied) {
                        new_y = proposed_y;
                        moved = true;
                    }
                }
            }

            // Apply the move if we found a free tile
            if (moved) {
                member->x = new_x;
                member->y = new_y;
                clamp_unit_position(member, field);
            }
        }
    }
}

bool should_use_formation(Unit* u) {
    // Only infantry, elites, officers, light gunners for formations
    switch (u->type) {
        case UNIT_CLONE_TROOPER:
        case UNIT_ELITE_CLONE:
        case UNIT_CLONE_OFFICER:
        case UNIT_BATTLE_DROID:
        case UNIT_SUPER_BATTLE_DROID:
        case UNIT_DROID_OFFICER:
        case UNIT_LIGHT_CLONE_GUNNER:
            return true;
        default:
            return false;
    }
}

void set_republic_strategy(const float weights[20]) {
    // Tactical (indices 0-5)
    republic_strategy.tactical.advance_distance          = weights[0];
    republic_strategy.tactical.defend_distance           = weights[1];
    republic_strategy.tactical.flanking_bias             = weights[2];
    republic_strategy.tactical.use_explosive_threshold   = weights[3];
    republic_strategy.tactical.retreat_hp_ratio          = weights[4];
    republic_strategy.tactical.cohesion_threshold        = weights[5];

    // Strategic (indices 6-16)
    republic_strategy.strategic.aggression_bias          = weights[6];
    republic_strategy.strategic.aggression_curve         = weights[7];
    republic_strategy.strategic.target_priority          = weights[8];
    republic_strategy.strategic.ally_proximity           = weights[9];
    republic_strategy.strategic.threat_weight            = weights[10];
    republic_strategy.strategic.focus_fire_tendency      = weights[11];
    republic_strategy.strategic.flank_left_weight        = weights[12];
    republic_strategy.strategic.flank_right_weight       = weights[13];
    republic_strategy.strategic.breakthrough_weight      = weights[14];
    republic_strategy.strategic.fallback_weight          = weights[15];
    republic_strategy.strategic.edge_avoidance_weight    = weights[16];

    // Deployment (indices 17-19)
    republic_strategy.deployment.deploy_horizontal_spread = weights[17];
    republic_strategy.deployment.deploy_vertical_offset   = weights[18];
    republic_strategy.deployment.deploy_flanking_bias     = weights[19];
}

void set_separatist_strategy(const float weights[20]) {
    // Tactical (indices 0-7)
    separatist_strategy.tactical.advance_distance          = weights[0];
    separatist_strategy.tactical.defend_distance           = weights[1];
    separatist_strategy.tactical.flanking_bias             = weights[2];
    separatist_strategy.tactical.use_explosive_threshold   = weights[3];
    separatist_strategy.tactical.retreat_hp_ratio          = weights[4];
    separatist_strategy.tactical.cohesion_threshold        = weights[5];

    // Strategic (indices 8-17)
    separatist_strategy.strategic.aggression_bias          = weights[6];
    separatist_strategy.strategic.aggression_curve         = weights[7];
    separatist_strategy.strategic.target_priority          = weights[8];
    separatist_strategy.strategic.ally_proximity           = weights[9];
    separatist_strategy.strategic.threat_weight            = weights[10];
    separatist_strategy.strategic.focus_fire_tendency      = weights[11];
    separatist_strategy.strategic.flank_left_weight        = weights[12];
    separatist_strategy.strategic.flank_right_weight       = weights[13];
    separatist_strategy.strategic.breakthrough_weight      = weights[14];
    separatist_strategy.strategic.fallback_weight          = weights[15];
    separatist_strategy.strategic.edge_avoidance_weight      = weights[16];

    // Deployment (indices 18-20)
    separatist_strategy.deployment.deploy_horizontal_spread = weights[17];
    separatist_strategy.deployment.deploy_vertical_offset   = weights[18];
    separatist_strategy.deployment.deploy_flanking_bias     = weights[19];
}

// Load files
void load_strategy_from_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Error: Could not open strategy file '%s'\n", filename);
        return;
    }

    float rep_weights[22] = {0};
    float sep_weights[22] = {0};
    int rep_count = 0, sep_count = 0;
    int current_faction = -1; // 0 = Republic, 1 = Separatist
    char line[256];

    while (fgets(line, sizeof(line), f)) {
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        // Skip empty lines and comments
        if (line[0] == '#' || line[0] == '\0') continue;

        if (strstr(line, "[REPUBLIC]")) {
            current_faction = 0;
            continue;
        }
        if (strstr(line, "[SEPARATIST]")) {
            current_faction = 1;
            continue;
        }

        char key[64];
        float value;
        if (sscanf(line, "%s %f", key, &value) == 2) {
            if (current_faction == 0 && rep_count < 21) {
                rep_weights[rep_count++] = value;
            } else if (current_faction == 1 && sep_count < 21) {
                sep_weights[sep_count++] = value;
            }
        }
    }
    fclose(f);

    // Apply if we have complete genomes
    while (rep_count < 22) rep_weights[rep_count++] = 0.6f;
    while (sep_count < 22) sep_weights[sep_count++] = 0.6f;
    set_republic_strategy(rep_weights);
    set_separatist_strategy(sep_weights);
    printf("Loaded strategies from %s\n", filename);
}

// Change unit composition here
float evaluate_matchup(const float* rep_weights, const float* sep_weights, int num_battles) {
    // Backup current strategies
    FactionStrategy old_rep = republic_strategy;
    FactionStrategy old_sep = separatist_strategy;

    // Apply given genomes
    set_republic_strategy(rep_weights);
    set_separatist_strategy(sep_weights);

    int rep_wins = 0;
    int timeouts = 0;
    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);

        spawn_republic_forces(&field, default_republic_composition, default_republic_composition_count);
        spawn_separatist_forces(&field, default_separatist_composition, default_separatist_composition_count);
        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) rep_wins++;
        else if (winner == 2) {
            timeouts++;
        }
        cleanup_battlefield(&field);
    }

    // Restore old strategies
    republic_strategy = old_rep;
    separatist_strategy = old_sep;

    // Fitness = win rate - timeout penalty
    float win_rate = (float)rep_wins / num_battles;
    float timeout_rate = (float)timeouts / num_battles;
    float penalty_factor = 2.0f; // make sure to adjust this for the penalty factor
    float fitness = win_rate - (timeout_rate * penalty_factor);

    return fitness;
}