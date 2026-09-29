#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "battlefield.h"
#include "ai.h"
#include "strategy.h"
#include "utils.h"
#include "queries.h"

// Default force compositions
ForceComposition default_republic_composition[] = {
    {UNIT_CLONE_TROOPER,        8},
    {UNIT_ELITE_CLONE,          0},
    {UNIT_CLONE_OFFICER,        0},
    {UNIT_CLONE_SNIPER,         0},
    {UNIT_LIGHT_CLONE_GUNNER,   0},
    {UNIT_MEDIUM_CLONE_GUNNER,  0},
    {UNIT_HEAVY_CLONE_GUNNER,   0},
    {UNIT_CLONE_ROCKET,         0},
    {UNIT_SWAMP_SPEEDER,        0},
    {UNIT_BARC_SPEEDER,         0},
    {UNIT_SABER_TANK,           0}
};
int default_republic_composition_count = sizeof(default_republic_composition) / sizeof(default_republic_composition[0]);

ForceComposition default_separatist_composition[] = {
    {UNIT_BATTLE_DROID,         8},
    {UNIT_SUPER_BATTLE_DROID,   0},
    {UNIT_DROID_OFFICER,        0},
    {UNIT_DROID_SNIPER,         0},
    {UNIT_MEDIUM_DROID_GUNNER,  0},
    {UNIT_HEAVY_DROID_GUNNER,   0},
    {UNIT_DROID_ROCKET,         0},
    {UNIT_SPIDER_DROID,         0},
    {UNIT_STAP,                 0},
    {UNIT_AAT,                  0}
};
int default_separatist_composition_count = sizeof(default_separatist_composition) / sizeof(default_separatist_composition[0]);

bool is_unit_used_in_composition(UnitType type) {
    // Check Republic composition
    for (int i = 0; i < default_republic_composition_count; i++) {
        if (default_republic_composition[i].type == type && default_republic_composition[i].count > 0) {
            return true;
        }
    }
    // Check Separatist composition
    for (int i = 0; i < default_separatist_composition_count; i++) {
        if (default_separatist_composition[i].type == type && default_separatist_composition[i].count > 0) {
            return true;
        }
    }
    return false;
}

// Action tracking:
ActionCounts action_counts = {0};
int total_actions_across_units = 0;
int units_that_acted = 0;
BattleStats battle_stats = {0};
char battle_action_log[4096] = {0};
bool turn_based = false;
bool print_battle_summary_flag = false;

// Initialization
void resolve_overlaps(Battlefield* field) {
    // check if any units overlap
    bool any_overlap = false;
    for (int i = 0; i < field->unit_count && !any_overlap; i++) {
        Unit* a = field->units[i];
        if (a == NULL || a->hp <= 0) continue;
        for (int j = i + 1; j < field->unit_count; j++) {
            Unit* b = field->units[j];
            if (b == NULL || b->hp <= 0) continue;
            if (a->x == b->x && a->y == b->y) {
                any_overlap = true;
                break;
            }
        }
    }
    if (!any_overlap) return; // nothing to resolve

    int max_iterations = 200;
    while (any_overlap && max_iterations-- > 0) {
        any_overlap = false;
        for (int i = 0; i < field->unit_count; i++) {
            Unit* a = field->units[i];
            if (a == NULL || a->hp <= 0) continue;
            for (int j = i + 1; j < field->unit_count; j++) {
                Unit* b = field->units[j];
                if (b == NULL || b->hp <= 0) continue;
                if (a->x == b->x && a->y == b->y) {
                    any_overlap = true;

                    // Try to move b to an adjacent free tile (cardinal directions first)
                    bool moved = false;
                    int dx_attempts[] = {0, 0, -1, 1};
                    int dy_attempts[] = {-1, 1, 0, 0};
                    for (int d = 0; d < 4; d++) {
                        int nx = b->x + dx_attempts[d];
                        int ny = b->y + dy_attempts[d];
                        if (is_tile_walkable(field, nx, ny, b)) {
                            b->x = nx;
                            b->y = ny;
                            moved = true;
                            i = -1; // restart outer loop
                            break;
                        }
                    }

                    // If cardinal directions blocked, try diagonals
                    if (!moved) {
                        int dx_diag[] = {-1, -1, 1, 1};
                        int dy_diag[] = {-1, 1, -1, 1};
                        for (int d = 0; d < 4; d++) {
                            int nx = b->x + dx_diag[d];
                            int ny = b->y + dy_diag[d];
                            if (is_tile_walkable(field, nx, ny, b)) {
                                b->x = nx;
                                b->y = ny;
                                moved = true;
                                i = -1;
                                break;
                            }
                        }
                    }

                    // If still blocked, expand radius gradually (max 3)
                    if (!moved) {
                        for (int radius = 2; radius <= 3 && !moved; radius++) {
                            for (int dy = -radius; dy <= radius && !moved; dy++) {
                                for (int dx = -radius; dx <= radius && !moved; dx++) {
                                    if (dx == 0 && dy == 0) continue;
                                    int nx = b->x + dx;
                                    int ny = b->y + dy;
                                    if (is_tile_walkable(field, nx, ny, b)) {
                                        b->x = nx;
                                        b->y = ny;
                                        moved = true;
                                        i = -1;
                                    }
                                }
                            }
                        }
                    }

                    // Last resort: random placement (limited attempts)
                    if (!moved) {
                        for (int attempt = 0; attempt < 30; attempt++) {
                            int nx = roll(0, field->width - 1);
                            int ny = roll(0, field->height - 1);
                            if (is_tile_walkable(field, nx, ny, b)) {
                                b->x = nx;
                                b->y = ny;
                                moved = true;
                                i = -1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void initialize_battlefield(Battlefield* field) {
    // Set dimensions
    field->width = MAX_COLS;
    field->height = MAX_ROWS;
    field->unit_count = 0;

    // Clear all units
    for (int i = 0; i < MAX_UNITS; i++) {
        field->units[i] = NULL;
    }

    // Populate the battlefield with open terrain
    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            field->battlefield[i][j] = TERRAIN_OPEN;
        }
    }
}

void spawn_republic_forces(Battlefield* field, ForceComposition forces[], int force_count) {
    int id = field->unit_count + 1;
    int y_min = field->height * 3 / 4;
    int y_max = field->height - 1;
    int x_min = 0;
    int x_max = field->width - 1;

    FactionStrategy* strat = &republic_strategy;

    int total_units = 0;
    for (int f = 0; f < force_count; f++) {
        total_units += forces[f].count;
    }
    if (total_units == 0) return;

    float center_x = (float)field->width / 2.0f;
    float flank_shift = (strat->deployment.deploy_flanking_bias / 10.0f) * (float)field->width * 0.25f;
    center_x += flank_shift;
    if (center_x < 0.0f) center_x = 0.0f;
    if (center_x >= field->width) center_x = field->width - 1.0f;

    float horizontal_spread = strat->deployment.deploy_horizontal_spread / 10.0f;
    float vertical_offset = strat->deployment.deploy_vertical_offset / 10.0f;
    int y_center = y_min + (int)((y_max - y_min) * vertical_offset);
    if (y_center < y_min) y_center = y_min;
    if (y_center > y_max) y_center = y_max;

    int unit_index = 0;
    for (int f = 0; f < force_count; f++) {
        for (int i = 0; i < forces[f].count; i++) {
            if (field->unit_count >= MAX_UNITS) break;
            float fraction = (total_units == 1) ? 0.5f : (float)unit_index / (total_units - 1);
            float x_offset = (fraction - 0.5f) * 2.0f * horizontal_spread * (float)field->width * 0.4f;
            int desired_x = (int)(center_x + x_offset);
            float y_offset = (fraction - 0.5f) * (y_max - y_min) * 0.3f;
            int desired_y = y_center + (int)y_offset;

            if (desired_x < 0) desired_x = 0;
            if (desired_x >= field->width) desired_x = field->width - 1;
            if (desired_y < y_min) desired_y = y_min;
            if (desired_y > y_max) desired_y = y_max;

            int x = desired_x, y = desired_y;
            bool placed = false;
            
            // try the desired tile first
            if (is_tile_walkable(field, x, y, NULL)) {
                placed = true;
            }

            if (!placed) {
                for (int radius = 1; radius <= 3 && !placed; radius++) {
                    for (int dy = -radius; dy <= radius && !placed; dy++) {
                        for (int dx = -radius; dx <= radius && !placed; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int tx = desired_x + dx;
                            int ty = desired_y + dy;
                            if (tx < 0 || tx >= field->width || ty < y_min || ty > y_max) continue;
                            if (is_tile_walkable(field, tx, ty, NULL)) {
                                x = tx; y = ty; placed = true;
                            }
                        }
                    }
                }
            }

            // If still not placed, do a random search
            if (!placed) {
                for (int attempt = 0; attempt < 100; attempt++) {
                    x = roll(x_min, x_max);
                    y = roll(y_min, y_max);
                    if (is_tile_walkable(field, x, y, NULL)) {
                        placed = true; break;
                    }
                }
            }

            // If there is still no placement, force it.
            if (!placed) {
                x = (unit_index * 3) % field->width;
                y = y_min + (unit_index % (y_max - y_min + 1));
                // If still not walkable, search linearly (rare)
                for (int yy = y_min; yy <= y_max && !placed; yy++) {
                    for (int xx = 0; xx < field->width && !placed; xx++) {
                        if (is_tile_walkable(field, xx, yy, NULL)) {
                            x = xx; y = yy; placed = true;
                        }
                    }
                }
            }

            Unit* squad = create_unit_squad(forces[f].type, id++, x, y);
            static int next_squad_id = 1;
            if (squad) {
                field->units[field->unit_count++] = squad;
                squad->squad_id = f;
                squad->formation_index = i;
                next_squad_id++;
            }
            unit_index++;
            resolve_overlaps(field);
        }
    }
}

void spawn_separatist_forces(Battlefield* field, ForceComposition forces[], int force_count) {
    int id = field->unit_count + 1;
    int y_min = 0;
    int y_max = field->height / 4 - 1;
    int x_min = 0;
    int x_max = field->width - 1;

    FactionStrategy* strat = &separatist_strategy;

    int total_units = 0;
    for (int f = 0; f < force_count; f++) {
        total_units += forces[f].count;
    }
    if (total_units == 0) return;

    float center_x = (float)field->width / 2.0f;
    float flank_shift = (strat->deployment.deploy_flanking_bias / 10.0f) * (float)field->width * 0.25f;
    center_x += flank_shift;
    if (center_x < 0.0f) center_x = 0.0f;
    if (center_x >= field->width) center_x = field->width - 1.0f;

    float horizontal_spread = strat->deployment.deploy_horizontal_spread / 10.0f;
    float vertical_offset = strat->deployment.deploy_vertical_offset / 10.0f;
    int y_center = y_min + (int)((y_max - y_min) * vertical_offset);
    if (y_center < y_min) y_center = y_min;
    if (y_center > y_max) y_center = y_max;

    int unit_index = 0;
    for (int f = 0; f < force_count; f++) {
        for (int i = 0; i < forces[f].count; i++) {
            if (field->unit_count >= MAX_UNITS) break;
            float fraction = (total_units == 1) ? 0.5f : (float)unit_index / (total_units - 1);
            float x_offset = (fraction - 0.5f) * 2.0f * horizontal_spread * (float)field->width * 0.4f;
            int desired_x = (int)(center_x + x_offset);
            float y_offset = (fraction - 0.5f) * (y_max - y_min) * 0.3f;
            int desired_y = y_center + (int)y_offset;

            if (desired_x < 0) desired_x = 0;
            if (desired_x >= field->width) desired_x = field->width - 1;
            if (desired_y < y_min) desired_y = y_min;
            if (desired_y > y_max) desired_y = y_max;

            int x = desired_x, y = desired_y;
            bool placed = false;
            
            if (is_tile_walkable(field, x, y, NULL)) {
                placed = true;
            }

            // If not, spiral search
            if (!placed) {
                for (int radius = 1; radius <= 3 && !placed; radius++) {
                    for (int dy = -radius; dy <= radius && !placed; dy++) {
                        for (int dx = -radius; dx <= radius && !placed; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int tx = desired_x + dx;
                            int ty = desired_y + dy;
                            if (tx < 0 || tx >= field->width || ty < y_min || ty > y_max) continue;
                            if (is_tile_walkable(field, tx, ty, NULL)) {
                                x = tx; y = ty; placed = true;
                            }
                        }
                    }
                }
            }

            // If still not placed, random search
            if (!placed) {
                for (int attempt = 0; attempt < 100; attempt++) {
                    x = roll(x_min, x_max);
                    y = roll(y_min, y_max);
                    if (is_tile_walkable(field, x, y, NULL)) {
                        placed = true; break;
                    }
                }
            }

            // If still nothing, force deploy
            if (!placed) {
                x = (unit_index * 3) % field->width;
                y = y_min + (unit_index % (y_max - y_min + 1));
                // If still not walkable, search linearly (rare)
                for (int yy = y_min; yy <= y_max && !placed; yy++) {
                    for (int xx = 0; xx < field->width && !placed; xx++) {
                        if (is_tile_walkable(field, xx, yy, NULL)) {
                            x = xx; y = yy; placed = true;
                        }
                    }
                }
            }

            Unit* squad = create_unit_squad(forces[f].type, id++, x, y);
            static int next_squad_id = 1;
            if (squad) {
                field->units[field->unit_count++] = squad;
                squad->squad_id = f;
                squad->formation_index = i;
                next_squad_id++;
            }
            unit_index++;
            resolve_overlaps(field);
        }
    }
}


// Start of Battle
int run_battle(Battlefield* field, int max_turns) {
    turn_count = 0;
    battle_action_log[0] = '\0';

    memset(&battle_stats, 0, sizeof(BattleStats));

    battle_stats.winner = 2;

    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        battle_stats.kills_by_type[i] = 0;
        battle_stats.casualties_by_type[i] = 0;
    }

    while (turn_count < max_turns) {
        int republic_count = 0, separatist_count = 0;
        for (int i = 0; i < field->unit_count; i++) {
            if (field->units[i]->hp <= 0) continue;
            if (field->units[i]->faction == FACTION_REPUBLIC) republic_count++;
            else separatist_count++;
        }
        if (republic_count == 0) return 1;
        if (separatist_count == 0) return 0;

        for (int i = 0; i < field->unit_count; i++) {
            if (field->units[i]->hp > 0) {
                field->units[i]->has_acted = false;
                field->units[i]->squad_moved = false;
            }
        }

        // Assign orders to both factions
        assign_faction_orders(field, FACTION_REPUBLIC);
        assign_faction_orders(field, FACTION_SEPARATIST);

        bool republic_turn = (roll(1, 2) == 1);
        int acted_this_turn = 0;
        int total_active = republic_count + separatist_count;

        update_strategic_orders(field);

        bool toggled_once = false;  // track if we already tried both sides

        int safety = 0; // infinite loop prevention

        while (acted_this_turn < total_active && safety++ < total_active * 2) {
            Faction current_faction = republic_turn ? FACTION_REPUBLIC : FACTION_SEPARATIST;
            Unit* acting_unit = select_closest_unit(field, current_faction);

            if (!acting_unit) {
                // If we've already toggled and still no unit, all units have acted
                if (toggled_once) break;
                republic_turn = !republic_turn;
                toggled_once = true;
                continue;
            }
            toggled_once = false;  // reset toggled flag when we find a unit

            acting_unit->has_acted = true;
            unit_turn(acting_unit, field, NULL);
            acted_this_turn++;

            republic_turn = !republic_turn;

            if (turn_based) {
                print_screen(field);
                if (strlen(battle_action_log) > 0) {
                    printf("%s", battle_action_log);
                    battle_action_log[0] = '\0';
                }
                printf("Press any key to advance... ");
                char press_next = getchar();
                if (press_next) printf("\n");
            }

            // Victory check
            republic_count = 0;
            separatist_count = 0;
            for (int i = 0; i < field->unit_count; i++) {
                if (field->units[i]->hp <= 0) continue;
                if (field->units[i]->faction == FACTION_REPUBLIC) republic_count++;
                else separatist_count++;
            }
            if (republic_count == 0) {
                battle_stats.winner = 1;
                if (turn_based) print_battle_summary_and_wait();
                return 1;
            }
            if (separatist_count == 0) {
                battle_stats.winner = 0;
                if (turn_based) print_battle_summary_and_wait();
                return 0;
            }

            // Update total active units
            republic_count = 0;
            separatist_count = 0;
            for (int i = 0; i < field->unit_count; i++) {
                if (field->units[i]->hp <= 0) continue;
                if (field->units[i]->faction == FACTION_REPUBLIC) republic_count++;
                else separatist_count++;
            }
            if (republic_count == 0) {
                battle_stats.winner = 1;
                if (turn_based) print_battle_summary_and_wait();
                return 1;
            }
            if (separatist_count == 0) {
                battle_stats.winner = 0;
                if (turn_based) print_battle_summary_and_wait();
                return 0;
            }

            // Update total_active to reflect remaining living units
            total_active = republic_count + separatist_count;
        }

        // something went wrong, force an exit
        if (safety >= total_active * 2) {
            break;
        }

        resolve_overlaps(field);
        turn_count++;
    }

    battle_stats.winner = 2;
    if (turn_based) print_battle_summary_and_wait();
    return 2;
}


// Cleanup
void cleanup_battlefield(Battlefield* field) {
    // Free all units (no memory leaks pleaseeeee)
    for (int i = 0; i < field->unit_count; i++) {
        free(field->units[i]);
        field->units[i] = NULL;
    }
    field->unit_count = 0;
}

void reset_stats() {
    total_actions_across_units = 0;
    units_that_acted = 0;
    
    // Reset action_counts
    action_counts.fire = 0;
    action_counts.advance_fire = 0;
    action_counts.explosive = 0;
    action_counts.rally = 0;
    action_counts.dash = 0;
    action_counts.retreat = 0;
    action_counts.total = 0;
}

void print_action_stats() {
    if (action_counts.total == 0) {
        printf("No actions tracked.\n");
        return;
    }
    
    printf("\n=== ACTION ECONOMY ===\n");
    printf("Total actions tracked: %d\n", action_counts.total);
    printf("\n");
    
    // Categories
    int aggressive = action_counts.fire + action_counts.advance_fire + action_counts.explosive;
    int movement = action_counts.dash;
    int defensive = action_counts.rally + action_counts.retreat;
    
    printf("Category breakdown:\n");
    printf("  Aggressive: %.1f%% (%d)\n", 
           (aggressive * 100.0f) / action_counts.total, aggressive);
    printf("  Movement:   %.1f%% (%d)\n", 
           (movement * 100.0f) / action_counts.total, movement);
    printf("  Defensive:  %.1f%% (%d)\n", 
           (defensive * 100.0f) / action_counts.total, defensive);
    printf("\n");
    
    printf("Detailed breakdown:\n");
    printf("  Fire:                 %.1f%%\n", 
           (action_counts.fire * 100.0f) / action_counts.total);
    printf("  Advance+Fire:         %.1f%%\n", 
           (action_counts.advance_fire * 100.0f) / action_counts.total);
    printf("  Explosive:            %.1f%%\n", 
           (action_counts.explosive * 100.0f) / action_counts.total);
    printf("  Dash:                 %.1f%%\n", 
           (action_counts.dash * 100.0f) / action_counts.total);
    printf("  Rally:                %.1f%%\n", 
           (action_counts.rally * 100.0f) / action_counts.total);
    printf("  Retreat:              %.1f%%\n", 
           (action_counts.retreat * 100.0f) / action_counts.total);
    printf("\n");
}

void print_battle_summary() {
    printf("\n=== BATTLE SUMMARY ===\n");
    printf("Winner: %s\n", (battle_stats.winner == 0) ? "Republic" : (battle_stats.winner == 1) ? "Separatist" : "Timeout");
    printf("Turns: %d\n", turn_count);
    printf("Shots fired: %d\n", battle_stats.shots_fired);
    printf("Shots hit: %d\n", battle_stats.shots_hit);
    printf("Kills: %d\n", battle_stats.kills);
    printf("Explosives used: %d\n", battle_stats.explosives_used);
    printf("Damage dealt: Republic %d, Separatist %d\n", battle_stats.damage_dealt_rep, battle_stats.damage_dealt_sep);
    printf("Casualties: Republic %d, Separatist %d\n", battle_stats.casualties_rep, battle_stats.casualties_sep);
    // Print kills by type
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        printf("  %s: %d kills\n", get_unit_type_name((UnitType)i), battle_stats.kills_by_type[i]);
    }
    printf("\n");
}

void print_battle_summary_and_wait(void) {
    if (turn_based) {
        print_battle_summary();
        printf("Press any key to continue... ");
        char press_next = getchar();
        if (press_next) printf("\n");
    }
}